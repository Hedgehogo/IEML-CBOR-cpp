#pragma once
#include <IEML/node.hpp>
#include <cbor/Decoder/Decoder.hpp>
#include "exception/FailedParseException.hpp"

namespace ieml_cbor {
	struct FileInclude;
	
	template<typename FileInclude_ = FileInclude>
	class BasicParser {
	private:
		cbor::Decoder decoder_;
		ieml::RcPtr<ieml::AnchorKeeper> anchorKeeper_;
		ieml::FilePath filePath_;
		
		cbor::DecoderState getState();
		
		template<cbor::DecoderState State>
		void checkState();
		
		void checkBytes();
		
		void decodeType();
		
		bool decodeStringSize();
		
	public:
		BasicParser(cbor::Input& input, ieml::RcPtr<ieml::AnchorKeeper> anchorKeeper, ieml::FilePath filePath = ieml::FilePath{});
		
		ieml::NodeData parse();
		
		ieml::NodeData parseNode();
		
		ieml::NullData parseNull();
		
		ieml::RawData parseRaw();
		
		ieml::StringData parseString();
		
		ieml::ListData parseList();
		
		ieml::MapData parseMap();
		
		ieml::TagData parseTag();
		
		ieml::FileData parseFile();
		
		ieml::TakeAnchorData parseTakeAnchor();
		
		ieml::GetAnchorData parseGetAnchor();
	};
	
	using Parser = BasicParser<>;
	
	struct FileInclude {
		static ieml::NodeData include(const ieml::RcPtr<ieml::AnchorKeeper>& anchorKeeper, const ieml::FilePath& filePath);
	};
	
	template<typename FileInclude_ = FileInclude>
	ieml::Node fromCBOR(cbor::Input& input, const ieml::RcPtr<ieml::AnchorKeeper>& anchorKeeper = ieml::makeRcPtr<ieml::AnchorKeeper>());
	
	template<typename FileInclude_ = FileInclude>
	ieml::Node fromCBORFile(const ieml::FilePath& filePath, const ieml::RcPtr<ieml::AnchorKeeper>& anchorKeeper = ieml::makeRcPtr<ieml::AnchorKeeper>());
}

#include "Parser.inl"
