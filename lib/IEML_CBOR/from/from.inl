#include "from.hpp"
#include "IEML_CBOR/from/Parser/Parser.hpp"


namespace ieml_cbor {
	template<typename FileInclude_>
	auto from_cbor(cbor::Input& input, ieml::RcPtr<ieml::AnchorKeeper> const& anchor_keeper) -> ieml::Node {
		BasicParser<FileInclude_> parser{input, anchor_keeper};
		return ieml::Node{ieml::FileData{parser.parse(), {}, anchor_keeper}};
	}
	
	template<typename FileInclude_>
	auto from_cbor_file(ieml::FilePath const& file_path, ieml::RcPtr<ieml::AnchorKeeper> const& anchor_keeper) -> ieml::Node {
		return ieml::Node{ieml::FileData{FileInclude_::include(anchor_keeper, file_path), file_path, anchor_keeper}};
	}
}