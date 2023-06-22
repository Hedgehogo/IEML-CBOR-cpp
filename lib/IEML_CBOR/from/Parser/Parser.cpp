#include "Parser.hpp"

namespace ieml_cbor {
	ieml::NodeData FileInclude::include(const ieml::RcPtr<ieml::AnchorKeeper>& anchorKeeper, const ieml::FilePath& filePath) {
		std::ifstream stream{filePath, std::ios::in | std::ios::binary};
		std::streamsize size{static_cast<std::streamsize>(std::filesystem::file_size(filePath))};
		std::vector<char> data(size);
		stream.read(data.data(), size);
		stream.close();

		cbor::Input input{data.data(), static_cast<int>(size)};
		BasicParser parser{input, anchorKeeper, filePath};
		return parser.parse();
	}
}