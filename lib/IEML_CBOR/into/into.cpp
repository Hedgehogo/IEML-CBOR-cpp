#include "into.hpp"

namespace ieml_cbor {
	void FileGenerate::generate(const ieml::Node& node, const ieml::FilePath& file_path) {
		cbor::OutputDynamic output{};
		cbor::Encoder encoder{output};
		intoCBOR<FileGenerate>(encoder, node);
		
		std::ofstream stream{file_path, std::ios::out | std::ios::binary};
		stream.write(reinterpret_cast<char*>(output.data()), output.size());
		stream.close();
	}
}