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
		ieml::RcPtr<ieml::AnchorKeeper> anchor_keeper_;
		ieml::FilePath file_path_;
		
		cbor::DecoderState get_state();
		
		template<cbor::DecoderState State>
		void check_state();
		
		void check_bytes();
		
		void decode_type();
		
		bool decode_string_size();
		
	public:
		BasicParser(cbor::Input& input, ieml::RcPtr<ieml::AnchorKeeper> anchor_keeper, ieml::FilePath file_path = ieml::FilePath{});
		
		ieml::NodeData parse();
		
		ieml::NodeData parse_node();
		
		ieml::NullData parse_null();
		
		ieml::RawData parse_raw();
		
		ieml::StringData parse_string();
		
		ieml::ListData parse_list();
		
		ieml::MapData parse_map();
		
		ieml::TagData parse_tag();
		
		ieml::FileData parse_file();
		
		ieml::TakeAnchorData parse_take_anchor();
		
		ieml::GetAnchorData parse_get_anchor();
	};
	
	using Parser = BasicParser<>;
	
	struct FileInclude {
		static ieml::NodeData include(const ieml::RcPtr<ieml::AnchorKeeper>& anchor_keeper, const ieml::FilePath& file_path);
	};
	
	template<typename FileInclude_ = FileInclude>
	ieml::Node from_cbor(cbor::Input& input, const ieml::RcPtr<ieml::AnchorKeeper>& anchor_keeper = ieml::make_rc_ptr<ieml::AnchorKeeper>());
	
	template<typename FileInclude_ = FileInclude>
	ieml::Node from_cbor_file(const ieml::FilePath& file_path, const ieml::RcPtr<ieml::AnchorKeeper>& anchor_keeper = ieml::make_rc_ptr<ieml::AnchorKeeper>());
}

#include "Parser.inl"
