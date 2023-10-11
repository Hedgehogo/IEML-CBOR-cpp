#include "Parser.hpp"

namespace ieml_cbor {
	ieml::NodeData FileInclude::include(const ieml::RcPtr<ieml::AnchorKeeper>& anchor_keeper, const ieml::FilePath& file_path) {
		std::ifstream stream{file_path, std::ios::in | std::ios::binary};
		std::streamsize size{static_cast<std::streamsize>(std::filesystem::file_size(file_path))};
		std::vector<char> data(size);
		stream.read(data.data(), size);
		stream.close();

		cbor::Input input{data.data(), static_cast<int>(size)};
		BasicParser parser{input, anchor_keeper, file_path};
		return parser.parse();
	}
}