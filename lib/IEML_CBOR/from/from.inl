#include "from.hpp"
#include "IEML_CBOR/from/Parser/Parser.hpp"


namespace ieml_cbor {
	template<typename FileInclude_>
	ieml::Node from_cbor(cbor::Input& input, const ieml::RcPtr<ieml::AnchorKeeper>& anchor_keeper) {
		BasicParser<FileInclude_> parser{input, anchor_keeper};
		return ieml::Node{ieml::FileData{parser.parse(), {}, anchor_keeper}};
	}
	
	template<typename FileInclude_>
	ieml::Node from_cbor_file(const ieml::FilePath& file_path, const ieml::RcPtr<ieml::AnchorKeeper>& anchor_keeper) {
		return ieml::Node{ieml::FileData{FileInclude_::include(anchor_keeper, file_path), file_path, anchor_keeper}};
	}
}