#pragma once
#include "IEML/node.hpp"
#include "cbor/Encoder/Encoder.hpp"

namespace ieml_cbor {
	struct FileGenerate {
		static void generate(const ieml::Node& node, const ieml::FilePath& file_path);
	};
	
	template<typename FileGenerate_ = FileGenerate>
	void into_cbor(cbor::Encoder& encoder, const ieml::Node& node);
	
	template<typename FileGenerate_ = FileGenerate>
	void into_cbor_file(const ieml::FilePath& file_path, const ieml::Node& node);
}

#include "into.inl"
