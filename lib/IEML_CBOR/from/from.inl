#include "from.hpp"
#include "IEML_CBOR/from/Parser/Parser.hpp"


namespace ieml_cbor {
	template<typename FileInclude_>
	ieml::Node fromCBOR(cbor::Input& input, const ieml::RcPtr<ieml::AnchorKeeper>& anchorKeeper) {
		BasicParser<FileInclude_> parser{input, anchorKeeper};
		return ieml::Node{ieml::FileData{parser.parse(), {}, anchorKeeper}};
	}
	
	template<typename FileInclude_>
	ieml::Node fromCBORFile(const ieml::FilePath& filePath, const ieml::RcPtr<ieml::AnchorKeeper>& anchorKeeper) {
		return ieml::Node{ieml::FileData{FileInclude_::include(anchorKeeper, filePath), filePath, anchorKeeper}};
	}
}