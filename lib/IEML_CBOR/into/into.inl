#include "cbor/OutputDynamic/OutputDynamic.hpp"

namespace ieml_cbor {
	namespace detail {
		template<typename FileGenerate_>
		void intoCBORItem(cbor::Encoder& encoder, const ieml::Node& node) {
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
					auto list{node.get_list()};
					encoder.write_array(static_cast<int>(list.ok().size()));
					for(const auto& iter: list.ok()) {
						intoCBOR(encoder, iter);
					}
				}
					break;
				case ieml::NodeType::Map: {
					auto map{node.get_map()};
					encoder.write_map(static_cast<int>(map.ok().size()));
					for(const auto& iter: map.ok()) {
						encoder.write_string(iter.first);
						intoCBOR(encoder, iter.second);
					}
				}
					break;
				case ieml::NodeType::Tag: {
					auto tag{node.get_tag()};
					encoder.write_array(2);
					encoder.write_string(tag.some());
					intoCBOR(encoder, node.get_clear_tag());
				}
					break;
				case ieml::NodeType::File: {
					auto file_path{node.get_file_path()};
					encoder.write_array(2);
					encoder.write_string(file_path.some().string());
					
					auto& anchors{node.get_file_anchor_keeper().some().get_file_map()};
					encoder.write_map(static_cast<int>(anchors.size()));
					for(const auto& anchor: anchors) {
						encoder.write_string(anchor.first);
						intoCBOR(encoder, anchor.second);
					}
					
					intoCBORFile(file_path.some(), node.get_clear_file());
				}
					break;
				case ieml::NodeType::TakeAnchor: {
					auto name{node.get_take_anchor_name()};
					encoder.write_array(2);
					encoder.write_string(name.some());
					intoCBOR(encoder, node.get_clear_take_anchor());
				}
					break;
				case ieml::NodeType::GetAnchor: {
					auto name{node.get_get_anchor_name()};
					encoder.write_string(name.some());
				}
					break;
			}
		}
	}
	
	template<typename FileGenerate_>
	void intoCBOR(cbor::Encoder& encoder, const ieml::Node& node) {
		detail::intoCBORItem<FileGenerate_>(encoder, node.get_clear_file());
	}
	
	template<typename FileGenerate_>
	void intoCBORFile(const ieml::FilePath& file_path, const ieml::Node& node) {
		FileGenerate_::generate(node, file_path);
	}
}