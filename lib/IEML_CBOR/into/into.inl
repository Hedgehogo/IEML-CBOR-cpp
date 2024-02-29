#include "cbor/OutputDynamic/OutputDynamic.hpp"

namespace ieml_cbor {
	namespace detail {
		template<typename FileGenerate_>
		auto into_cbor_item(cbor::Encoder& encoder, ieml::Node const& node) -> void {
			auto type{node.get_type()};
			encoder.write_array(2);
			encoder.write_int(static_cast<std::int32_t>(ieml::get_index_from_node_type(type)));
			switch(type) {
				case ieml::NodeType::Null:
					encoder.write_null();
					break;
				case ieml::NodeType::Raw:
					encoder.write_string(node.get_raw().ok().str);
					break;
				case ieml::NodeType::String:
					encoder.write_string(node.get_string().ok());
					break;
				case ieml::NodeType::List: {
					auto& list{node.get_list().ok()};
					encoder.write_array(static_cast<int>(list.size()));
					for(const auto& iter: list) {
						into_cbor(encoder, iter);
					}
				}
					break;
				case ieml::NodeType::Map: {
					auto& map{node.get_map().ok()};
					encoder.write_map(static_cast<int>(map.size()));
					for(const auto& iter: map) {
						encoder.write_string(iter.first);
						into_cbor(encoder, iter.second);
					}
				}
					break;
				case ieml::NodeType::Tag: {
					auto& tag{node.get_tag().some()};
					encoder.write_array(2);
					encoder.write_string(tag);
					into_cbor(encoder, node.get_clear_tag());
				}
					break;
				case ieml::NodeType::File: {
					auto& file_path{node.get_file_path().some()};
					encoder.write_array(2);
					encoder.write_string(file_path.string());
					
					auto& anchors{node.get_file_anchor_keeper().some().get_file_map()};
					encoder.write_map(static_cast<int>(anchors.size()));
					for(const auto& anchor: anchors) {
						encoder.write_string(anchor.first);
						into_cbor(encoder, anchor.second);
					}
					
					into_cbor_file(file_path, node.get_clear_file());
				}
					break;
				case ieml::NodeType::TakeAnchor: {
					auto& name{node.get_take_anchor_name().some()};
					encoder.write_array(2);
					encoder.write_string(name);
					into_cbor(encoder, node.get_clear_take_anchor());
				}
					break;
				case ieml::NodeType::GetAnchor: {
					auto& name{node.get_get_anchor_name().some()};
					encoder.write_string(name);
				}
					break;
			}
		}
	}
	
	template<typename FileGenerate_>
	auto into_cbor(cbor::Encoder& encoder, ieml::Node const& node) -> void {
		detail::into_cbor_item<FileGenerate_>(encoder, node.get_clear_file());
	}
	
	template<typename FileGenerate_>
	auto into_cbor_file(ieml::FilePath const& file_path, ieml::Node const& node) -> void {
		FileGenerate_::generate(node, file_path);
	}
}