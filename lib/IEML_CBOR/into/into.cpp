#include "into.hpp"

namespace ieml_cbor {
	auto FileGenerate::generate(ieml::Node const& node, ieml::FilePath const& file_path) -> void {
		auto output{cbor::OutputDynamic{}};
		auto encoder{cbor::Encoder{output}};
		into_cbor<FileGenerate>(encoder, node);
		
		auto stream{std::ofstream{file_path, std::ios::out | std::ios::binary}};
		stream.write(reinterpret_cast<char*>(output.data()), output.size());
		stream.close();
	}
}