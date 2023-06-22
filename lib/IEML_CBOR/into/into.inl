#include "cbor/OutputDynamic/OutputDynamic.hpp"

namespace ieml_cbor {
	namespace detail {
		template<typename FileGenerate_>
		void intoCBORItem(cbor::Encoder& encoder, const ieml::Node& node) {
			auto type{node.getType()};
			encoder.write_array(2);
			encoder.write_int(static_cast<std::int32_t>(ieml::getIndexFromNodeType(type)));
			switch(type) {
				case ieml::NodeType::Null:
					encoder.write_null();
					break;
				case ieml::NodeType::Raw:
					encoder.write_string(node.getRaw().ok().str);
					break;
				case ieml::NodeType::String:
					encoder.write_string(node.getString().ok());
					break;
				case ieml::NodeType::List: {
					auto list{node.getList()};
					encoder.write_array(static_cast<int>(list.ok().size()));
					for(const auto& iter: list.ok()) {
						intoCBOR(encoder, iter);
					}
				}
					break;
				case ieml::NodeType::Map: {
					auto map{node.getMap()};
					encoder.write_map(static_cast<int>(map.ok().size()));
					for(const auto& iter: map.ok()) {
						encoder.write_string(iter.first);
						intoCBOR(encoder, iter.second);
					}
				}
					break;
				case ieml::NodeType::Tag: {
					auto tag{node.getTag()};
					encoder.write_array(2);
					encoder.write_string(tag.some());
					intoCBOR(encoder, node.getClearTag());
				}
					break;
				case ieml::NodeType::File: {
					auto filePath{node.getFilePath()};
					encoder.write_array(2);
					encoder.write_string(filePath.some().string());
					
					auto& anchors{node.getFileAnchorKeeper().some().getFileMap()};
					encoder.write_map(static_cast<int>(anchors.size()));
					for(const auto& anchor: anchors) {
						encoder.write_string(anchor.first);
						intoCBOR(encoder, anchor.second);
					}
					
					intoCBORFile(filePath.some(), node.getClearFile());
				}
					break;
				case ieml::NodeType::TakeAnchor: {
					auto name{node.getTakeAnchorName()};
					encoder.write_array(2);
					encoder.write_string(name.some());
					intoCBOR(encoder, node.getClearTakeAnchor());
				}
					break;
				case ieml::NodeType::GetAnchor: {
					auto name{node.getGetAnchorName()};
					encoder.write_string(name.some());
				}
					break;
			}
		}
	}
	
	template<typename FileGenerate_>
	void intoCBOR(cbor::Encoder& encoder, const ieml::Node& node) {
		detail::intoCBORItem<FileGenerate_>(encoder, node.getClearFile());
	}
	
	template<typename FileGenerate_>
	void intoCBORFile(const ieml::FilePath& filePath, const ieml::Node& node) {
		FileGenerate_::generate(node, filePath);
	}
}