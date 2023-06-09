#include "into.hpp"

namespace ieml_cbor {
	void FileGenerate::generate(const ieml::Node& node, const ieml::FilePath& filePath) {
		cbor::OutputDynamic output{};
		cbor::Encoder encoder{output};
		intoCBOR<FileGenerate>(encoder, node);
		
		std::ofstream stream{filePath, std::ios::out | std::ios::binary};
		stream.write(reinterpret_cast<char*>(output.data()), output.size());
		stream.close();
	}
}