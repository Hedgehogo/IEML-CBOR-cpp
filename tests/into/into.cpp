#include <gtest/gtest.h>
#include <cbor/cbor.hpp>
#include <IEML_CBOR/from.hpp>
#include <IEML_CBOR/into.hpp>

template<typename FileInclude_ = ieml_cbor::FileInclude, typename FileGenerate_ = ieml_cbor::FileGenerate>
ieml::Node test_into(ieml::Node const& node, ieml::RcPtr<ieml::AnchorKeeper> const& anchor_keeper = ieml::make_rc_ptr<ieml::AnchorKeeper>()) {
	auto output{cbor::OutputDynamic{}};
	auto encoder{cbor::Encoder{output}};
	ieml_cbor::into_cbor<FileGenerate_>(encoder, ieml::FileData{node, {}, ieml::make_rc_ptr<ieml::AnchorKeeper>()});
	
	auto input{cbor::Input{output.data(), static_cast<int>(output.size())}};
	return ieml_cbor::from_cbor<FileInclude_>(input, anchor_keeper).get_clear_file();
}

TEST(into, null) {
	auto node{ieml::Node{ieml::NullData{}}};
	ASSERT_EQ(node, test_into(node));
}

TEST(into, raw) {
	auto node{ieml::Node{ieml::RawData{"hello"}}};
	ASSERT_EQ(node, test_into(node));
}

TEST(into, string) {
	auto node{ieml::Node{ieml::StringData{"hello"}}};
	ASSERT_EQ(node, test_into(node));
}

TEST(into, list) {
	auto node{ieml::Node{ieml::ListData{ieml::NullData{}, ieml::RawData{"hello"}}}};
	ASSERT_EQ(node, test_into(node));
}

TEST(into, map) {
	auto node{ieml::Node{ieml::MapData{
		{"first",  ieml::NullData{}},
		{"second", ieml::RawData{"hello"}}
	}}};
	ASSERT_EQ(node, test_into(node));
}

TEST(into, tag) {
	auto node{ieml::Node{ieml::TagData{ieml::NullData{}, "hello"}}};
	ASSERT_EQ(node, test_into(node));
}

static std::map<ieml::String, std::vector<uint8_t> > files{};

struct FileInclude {
	static ieml::NodeData include(ieml::RcPtr<ieml::AnchorKeeper> const& anchor_keeper, ieml::FilePath const& file_path) {
		auto input{cbor::Input{files[file_path.string()].data(), static_cast<int>(files[file_path.string()].size())}};
		auto parser{ieml_cbor::BasicParser<FileInclude>{input, anchor_keeper, file_path}};
		return parser.parse();
	}
};

struct FileGenerate {
	static void generate(ieml::Node const& node, ieml::FilePath const& file_path) {
		auto output{cbor::OutputDynamic{}};
		auto encoder{cbor::Encoder{output}};
		ieml_cbor::into_cbor(encoder, node);
		files.emplace(file_path.string(), std::vector<uint8_t>{output.data(), output.data() + output.size()});
	}
};

TEST(into, file) {
	auto output_anchor_keeper{ieml::make_rc_ptr<ieml::AnchorKeeper>()};
	output_anchor_keeper->add_to_file("anchor", ieml::NullData{});
	auto node{ieml::Node{ieml::FileData{ieml::NullData{}, ieml::FilePath{"test.cbor"}, output_anchor_keeper}}};
	
	ASSERT_EQ(node, (test_into<FileInclude, FileGenerate>(node)));
}

TEST(into, take_anchor) {
	auto output_anchor_keeper{ieml::make_rc_ptr<ieml::AnchorKeeper>()};
	output_anchor_keeper->add("anchor", ieml::RawData{"hello"});
	auto node{ieml::Node{ieml::TakeAnchorData{output_anchor_keeper, "anchor"}}};
	
	auto input_anchor_keeper{ieml::make_rc_ptr<ieml::AnchorKeeper>()};
	ASSERT_EQ(node, (test_into(node, input_anchor_keeper)));
	ASSERT_EQ(output_anchor_keeper->get_map(), input_anchor_keeper->get_map());
}

TEST(into, get_anchor) {
	auto output_anchor_keeper{ieml::make_rc_ptr<ieml::AnchorKeeper>()};
	output_anchor_keeper->add("anchor", ieml::RawData{"hello"});
	auto node{ieml::Node{ieml::GetAnchorData{output_anchor_keeper, "anchor"}}};
	
	ASSERT_EQ(node, (test_into(node, output_anchor_keeper)));
	ASSERT_EQ(node.get_clear_get_anchor(), *output_anchor_keeper->get("anchor"));
}