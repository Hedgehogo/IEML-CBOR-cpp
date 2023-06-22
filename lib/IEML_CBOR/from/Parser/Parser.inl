//included into Parser.hpp

#include <utility>

namespace ieml_cbor {
	template<typename FileInclude_>
	BasicParser<FileInclude_>::BasicParser(cbor::Input& input, ieml::RcPtr<ieml::AnchorKeeper> anchorKeeper, ieml::FilePath filePath) :
		decoder_(input), anchorKeeper_(std::move(anchorKeeper)), filePath_(std::move(filePath)) {
	}
	
	template<typename FileInclude_>
	cbor::DecoderState BasicParser<FileInclude_>::getState() {
		return decoder_.get_state();
	}
	
	template<typename FileInclude_>
	template<cbor::DecoderState State>
	void BasicParser<FileInclude_>::checkState() {
		if(decoder_.get_state() != State) {
			throw FailedParseException{FailedParseException::Reason::InvalidDocumentStructure};
		}
	}
	
	template<typename FileInclude_>
	void BasicParser<FileInclude_>::checkBytes() {
		if(!decoder_.has_bytes()) {
			throw FailedParseException{FailedParseException::Reason::IncompleteDocument};
		}
	}
	
	template<typename FileInclude_>
	void BasicParser<FileInclude_>::decodeType() {
		if(!decoder_.has_bytes(1)) {
			throw FailedParseException{FailedParseException::Reason::IncompleteDocument};
		}
		decoder_.decode_type();
	}
	
	template<typename FileInclude_>
	bool BasicParser<FileInclude_>::decodeStringSize() {
		if(getState() == cbor::DecoderState::StringSize) {
			decoder_.decode_string_size();
			checkBytes();
			decodeType();
			return true;
		}
		return false;
	}
	
	template<typename FileInclude_>
	ieml::NodeData BasicParser<FileInclude_>::parse() {
		return parseNode();
	}
	
	template<typename FileInclude_>
	ieml::NodeData BasicParser<FileInclude_>::parseNode() {
		decodeType();
		checkState<cbor::DecoderState::Array>();
		checkBytes();
		auto size{decoder_.decode_array_size()};
		if(size != 2) {
			throw FailedParseException{FailedParseException::Reason::InvalidDocumentStructure};
		}
		
		decodeType();
		checkState<cbor::DecoderState::PInt>();
		checkBytes();
		ieml::NodeType type{ieml::getNodeTypeFromIndex(decoder_.decode_p_int())};
		
		decodeType();
		checkBytes();
		switch(type) {
			case ieml::NodeType::Null:
				return {parseNull()};
			case ieml::NodeType::Raw:
				return {parseRaw()};
			case ieml::NodeType::String:
				return {parseString()};
			case ieml::NodeType::List:
				return {parseList()};
			case ieml::NodeType::Map:
				return {parseMap()};
			case ieml::NodeType::Tag:
				return {parseTag()};
			case ieml::NodeType::File:
				return {parseFile()};
			case ieml::NodeType::TakeAnchor:
				return {parseTakeAnchor()};
			case ieml::NodeType::GetAnchor:
				return {parseGetAnchor()};
		}
	}
	
	template<typename FileInclude_>
	ieml::NullData BasicParser<FileInclude_>::parseNull() {
		checkState<cbor::DecoderState::Null>();
		return {};
	}
	
	template<typename FileInclude_>
	ieml::RawData BasicParser<FileInclude_>::parseRaw() {
		if(decodeStringSize()) {
			checkBytes();
		}
		checkState<cbor::DecoderState::StringData>();
		return {decoder_.decode_string_data()};
	}
	
	template<typename FileInclude_>
	ieml::StringData BasicParser<FileInclude_>::parseString() {
		if(decodeStringSize()) {
			checkBytes();
		}
		checkState<cbor::DecoderState::StringData>();
		return decoder_.decode_string_data();
	}
	
	template<typename FileInclude_>
	ieml::ListData BasicParser<FileInclude_>::parseList() {
		checkState<cbor::DecoderState::Array>();
		auto size{decoder_.decode_array_size()};
		
		ieml::ListData list{};
		for(uint32_t i = 0; i < size; ++i) {
			list.emplace_back(parseNode());
		}
		
		return list;
	}
	
	template<typename FileInclude_>
	ieml::MapData BasicParser<FileInclude_>::parseMap() {
		checkState<cbor::DecoderState::Map>();
		auto size{decoder_.decode_map_size()};
		
		ieml::MapData map{};
		for(uint32_t i = 0; i < size; ++i) {
			decodeType();
			checkBytes();
			decodeStringSize();
			checkState<cbor::DecoderState::StringData>();
			checkBytes();
			ieml::String key{decoder_.decode_string_data()};
			
			map.emplace(key, parseNode());
		}
		
		return map;
	}
	
	template<typename FileInclude_>
	ieml::TagData BasicParser<FileInclude_>::parseTag() {
		checkState<cbor::DecoderState::Array>();
		if(decoder_.decode_array_size() != 2) {
			throw FailedParseException{FailedParseException::Reason::InvalidDocumentStructure};
		}
		
		decodeType();
		decodeStringSize();
		checkState<cbor::DecoderState::StringData>();
		checkBytes();
		ieml::String tag{decoder_.decode_string_data()};
		
		return {ieml::TagData{parseNode(), tag}};
	}
	
	template<typename FileInclude_>
	ieml::FileData BasicParser<FileInclude_>::parseFile() {
		checkState<cbor::DecoderState::Array>();
		if(decoder_.decode_array_size() != 2) {
			throw FailedParseException{FailedParseException::Reason::InvalidDocumentStructure};
		}
		
		decodeType();
		decodeStringSize();
		checkState<cbor::DecoderState::StringData>();
		checkBytes();
		ieml::FilePath filePath{decoder_.decode_string_data()};
		
		decodeType();
		checkState<cbor::DecoderState::Map>();
		checkBytes();
		uint32_t size{decoder_.decode_map_size()};
		
		ieml::RcPtr<ieml::AnchorKeeper> loadedAnchorKeeper{anchorKeeper_};
		for(uint32_t i = 0; i < size; ++i) {
			decodeType();
			decodeStringSize();
			checkState<cbor::DecoderState::StringData>();
			checkBytes();
			ieml::String key{decoder_.decode_string_data()};
			
			loadedAnchorKeeper->addToFile(key, parseNode());
		}
		
		return ieml::FileData{FileInclude_::include(loadedAnchorKeeper, filePath), filePath, loadedAnchorKeeper};
	}
	
	template<typename FileInclude_>
	ieml::TakeAnchorData BasicParser<FileInclude_>::parseTakeAnchor() {
		checkState<cbor::DecoderState::Array>();
		if(decoder_.decode_array_size() != 2) {
			throw FailedParseException{FailedParseException::Reason::InvalidDocumentStructure};
		}
		
		decodeType();
		decodeStringSize();
		checkState<cbor::DecoderState::StringData>();
		checkBytes();
		ieml::String name{decoder_.decode_string_data()};
		
		anchorKeeper_->add(name, parseNode());
		return {ieml::TakeAnchorData{anchorKeeper_, name}};
	}
	
	template<typename FileInclude_>
	ieml::GetAnchorData BasicParser<FileInclude_>::parseGetAnchor() {
		decodeStringSize();
		checkState<cbor::DecoderState::StringData>();
		checkBytes();
		return ieml::GetAnchorData{anchorKeeper_, decoder_.decode_string_data()};
	}
}