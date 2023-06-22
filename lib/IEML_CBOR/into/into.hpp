#pragma once
#include "IEML/node.hpp"
#include "cbor/Encoder/Encoder.hpp"

namespace ieml_cbor {
	struct FileGenerate {
		static void generate(const ieml::Node& node, const ieml::FilePath& filePath);
	};
	
	template<typename FileGenerate_ = FileGenerate>
	void intoCBOR(cbor::Encoder& encoder, const ieml::Node& node);
	
	template<typename FileGenerate_ = FileGenerate>
	void intoCBORFile(const ieml::FilePath& filePath, const ieml::Node& node);
}

#include "into.inl"
