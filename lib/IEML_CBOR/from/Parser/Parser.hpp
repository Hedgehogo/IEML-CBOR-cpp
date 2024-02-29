#pragma once
#include <IEML/node.hpp>
#include <cbor/Decoder/Decoder.hpp>
#include "exception/FailedParseException.hpp"

namespace ieml_cbor {
	struct FileInclude;
	
	template<typename FileInclude_ = FileInclude>
	class BasicParser {
	public:
		BasicParser(cbor::Input& input, ieml::RcPtr<ieml::AnchorKeeper> anchor_keeper, ieml::FilePath file_path = ieml::FilePath{});
		
		auto parse() -> ieml::NodeData;
		
		auto parse_node() -> ieml::NodeData;
		
		auto parse_null() -> ieml::NullData;
		
		auto parse_raw() -> ieml::RawData;
		
		auto parse_string() -> ieml::StringData;
		
		auto parse_list() -> ieml::ListData;
		
		auto parse_map() -> ieml::MapData;
		
		auto parse_tag() -> ieml::TagData;
		
		auto parse_file() -> ieml::FileData;
		
		auto parse_take_anchor() -> ieml::TakeAnchorData;
		
		auto parse_get_anchor() -> ieml::GetAnchorData;
	
	private:
		auto get_state() -> cbor::DecoderState;
		
		template<cbor::DecoderState State>
		auto check_state() -> void;
		
		auto check_bytes() -> void;
		
		auto decode_type() -> void;
		
		auto decode_string_size() -> bool;
		
		cbor::Decoder decoder_;
		ieml::RcPtr<ieml::AnchorKeeper> anchor_keeper_;
		ieml::FilePath file_path_;
	};
	
	using Parser = BasicParser<>;
	
	struct FileInclude {
		static auto include(ieml::RcPtr<ieml::AnchorKeeper> const& anchor_keeper, ieml::FilePath const& file_path) -> ieml::NodeData;
	};
	
	template<typename FileInclude_ = FileInclude>
	auto from_cbor(cbor::Input& input, ieml::RcPtr<ieml::AnchorKeeper> const& anchor_keeper = ieml::make_rc_ptr<ieml::AnchorKeeper>()) -> ieml::Node;
	
	template<typename FileInclude_ = FileInclude>
	auto from_cbor_file(
		ieml::FilePath const& file_path,
		ieml::RcPtr<ieml::AnchorKeeper> const& anchor_keeper = ieml::make_rc_ptr<ieml::AnchorKeeper>()
	) -> ieml::Node;
}

#include "Parser.inl"
