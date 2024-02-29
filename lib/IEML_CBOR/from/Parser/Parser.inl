//included into Parser.hpp

#include <utility>

namespace ieml_cbor {
	template<typename FileInclude_>
	BasicParser<FileInclude_>::BasicParser(cbor::Input& input, ieml::RcPtr<ieml::AnchorKeeper> anchor_keeper, ieml::FilePath file_path) :
		decoder_(input), anchor_keeper_(std::move(anchor_keeper)), file_path_(std::move(file_path)) {
	}
	
	template<typename FileInclude_>
	auto BasicParser<FileInclude_>::get_state() -> cbor::DecoderState {
		return decoder_.get_state();
	}
	
	template<typename FileInclude_>
	template<cbor::DecoderState State>
	auto BasicParser<FileInclude_>::check_state() -> void {
		if(decoder_.get_state() != State) {
			throw FailedParseException{FailedParseException::Reason::InvalidDocumentStructure};
		}
	}
	
	template<typename FileInclude_>
	auto BasicParser<FileInclude_>::check_bytes() -> void {
		if(!decoder_.has_bytes()) {
			throw FailedParseException{FailedParseException::Reason::IncompleteDocument};
		}
	}
	
	template<typename FileInclude_>
	auto BasicParser<FileInclude_>::decode_type() -> void {
		if(!decoder_.has_bytes(1)) {
			throw FailedParseException{FailedParseException::Reason::IncompleteDocument};
		}
		decoder_.decode_type();
	}
	
	template<typename FileInclude_>
	auto BasicParser<FileInclude_>::decode_string_size() -> bool {
		if(get_state() == cbor::DecoderState::StringSize) {
			decoder_.decode_string_size();
			check_bytes();
			decode_type();
			return true;
		}
		return false;
	}
	
	template<typename FileInclude_>
	auto BasicParser<FileInclude_>::parse() -> ieml::NodeData {
		return parse_node();
	}
	
	template<typename FileInclude_>
	auto BasicParser<FileInclude_>::parse_node() -> ieml::NodeData {
		decode_type();
		check_state<cbor::DecoderState::Array>();
		check_bytes();
		auto size{decoder_.decode_array_size()};
		if(size != 2) {
			throw FailedParseException{FailedParseException::Reason::InvalidDocumentStructure};
		}
		
		decode_type();
		check_state<cbor::DecoderState::PInt>();
		check_bytes();
		auto type{ieml::get_node_type_from_index(decoder_.decode_p_int())};
		
		decode_type();
		check_bytes();
		switch(type) {
			case ieml::NodeType::Null:
				return {parse_null()};
			case ieml::NodeType::Raw:
				return {parse_raw()};
			case ieml::NodeType::String:
				return {parse_string()};
			case ieml::NodeType::List:
				return {parse_list()};
			case ieml::NodeType::Map:
				return {parse_map()};
			case ieml::NodeType::Tag:
				return {parse_tag()};
			case ieml::NodeType::File:
				return {parse_file()};
			case ieml::NodeType::TakeAnchor:
				return {parse_take_anchor()};
			case ieml::NodeType::GetAnchor:
				return {parse_get_anchor()};
		}
	}
	
	template<typename FileInclude_>
	auto BasicParser<FileInclude_>::parse_null() -> ieml::NullData {
		check_state<cbor::DecoderState::Null>();
		return {};
	}
	
	template<typename FileInclude_>
	auto BasicParser<FileInclude_>::parse_raw() -> ieml::RawData {
		if(decode_string_size()) {
			check_bytes();
		}
		check_state<cbor::DecoderState::StringData>();
		return {decoder_.decode_string_data()};
	}
	
	template<typename FileInclude_>
	auto BasicParser<FileInclude_>::parse_string() -> ieml::StringData {
		if(decode_string_size()) {
			check_bytes();
		}
		check_state<cbor::DecoderState::StringData>();
		return decoder_.decode_string_data();
	}
	
	template<typename FileInclude_>
	auto BasicParser<FileInclude_>::parse_list() -> ieml::ListData {
		check_state<cbor::DecoderState::Array>();
		auto size{decoder_.decode_array_size()};
		
		ieml::ListData list{};
		for(uint32_t i = 0; i < size; ++i) {
			list.emplace_back(parse_node());
		}
		
		return list;
	}
	
	template<typename FileInclude_>
	auto BasicParser<FileInclude_>::parse_map() -> ieml::MapData {
		check_state<cbor::DecoderState::Map>();
		auto size{decoder_.decode_map_size()};
		
		ieml::MapData map{};
		for(uint32_t i = 0; i < size; ++i) {
			decode_type();
			check_bytes();
			decode_string_size();
			check_state<cbor::DecoderState::StringData>();
			check_bytes();
			ieml::String key{decoder_.decode_string_data()};
			
			map.emplace(key, parse_node());
		}
		
		return map;
	}
	
	template<typename FileInclude_>
	auto BasicParser<FileInclude_>::parse_tag() -> ieml::TagData {
		check_state<cbor::DecoderState::Array>();
		if(decoder_.decode_array_size() != 2) {
			throw FailedParseException{FailedParseException::Reason::InvalidDocumentStructure};
		}
		
		decode_type();
		decode_string_size();
		check_state<cbor::DecoderState::StringData>();
		check_bytes();
		auto tag{decoder_.decode_string_data()};
		
		return {ieml::TagData{parse_node(), std::move(tag)}};
	}
	
	template<typename FileInclude_>
	auto BasicParser<FileInclude_>::parse_file() -> ieml::FileData {
		check_state<cbor::DecoderState::Array>();
		if(decoder_.decode_array_size() != 2) {
			throw FailedParseException{FailedParseException::Reason::InvalidDocumentStructure};
		}
		
		decode_type();
		decode_string_size();
		check_state<cbor::DecoderState::StringData>();
		check_bytes();
		auto file_path{ieml::FilePath{decoder_.decode_string_data()}};
		
		decode_type();
		check_state<cbor::DecoderState::Map>();
		check_bytes();
		auto size{size_t{decoder_.decode_map_size()}};
		
		auto loaded_anchor_keeper{anchor_keeper_};
		for(uint32_t i = 0; i < size; ++i) {
			decode_type();
			decode_string_size();
			check_state<cbor::DecoderState::StringData>();
			check_bytes();
			ieml::String key{decoder_.decode_string_data()};
			
			loaded_anchor_keeper->add_to_file(key, parse_node());
		}
		
		return ieml::FileData{FileInclude_::include(loaded_anchor_keeper, file_path), file_path, loaded_anchor_keeper};
	}
	
	template<typename FileInclude_>
	auto BasicParser<FileInclude_>::parse_take_anchor() -> ieml::TakeAnchorData {
		check_state<cbor::DecoderState::Array>();
		if(decoder_.decode_array_size() != 2) {
			throw FailedParseException{FailedParseException::Reason::InvalidDocumentStructure};
		}
		
		decode_type();
		decode_string_size();
		check_state<cbor::DecoderState::StringData>();
		check_bytes();
		auto name{decoder_.decode_string_data()};
		
		anchor_keeper_->add(name, parse_node());
		return {ieml::TakeAnchorData{anchor_keeper_, name}};
	}
	
	template<typename FileInclude_>
	auto BasicParser<FileInclude_>::parse_get_anchor() -> ieml::GetAnchorData {
		decode_string_size();
		check_state<cbor::DecoderState::StringData>();
		check_bytes();
		return ieml::GetAnchorData{anchor_keeper_, decoder_.decode_string_data()};
	}
}