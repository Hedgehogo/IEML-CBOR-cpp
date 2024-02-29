#pragma once
#include "IEML/node.hpp"
#include "cbor/Encoder/Encoder.hpp"

namespace ieml_cbor {
	struct FileGenerate {
		static auto generate(ieml::Node const& node, ieml::FilePath const& file_path) -> void;
	};
	
	template<typename FileGenerate_ = FileGenerate>
	auto into_cbor(cbor::Encoder& encoder, ieml::Node const& node) -> void;
	
	template<typename FileGenerate_ = FileGenerate>
	auto into_cbor_file(ieml::FilePath const& file_path, ieml::Node const& node) -> void;
}

#include "into.inl"
