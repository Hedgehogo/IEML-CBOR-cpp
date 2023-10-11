#include <gtest/gtest.h>
#include <cbor/cbor.hpp>
#include <IEML_CBOR/from.hpp>
#include <IEML_CBOR/into.hpp>

template<typename FileInclude_ = ieml_cbor::FileInclude, typename FileGenerate_ = ieml_cbor::FileGenerate>
ieml::Node test_into(ieml::Node const& node, ieml::RcPtr<ieml::AnchorKeeper> const& anchor_keeper = ieml::make_rc_ptr<ieml::AnchorKeeper>()) {
	cbor::OutputDynamic output;
	cbor::Encoder encoder(output);
	ieml_cbor::into_cbor<FileGenerate_>(encoder, ieml::FileData{node, {}, ieml::make_rc_ptr<ieml::AnchorKeeper>()});
	
	cbor::Input input(output.data(), static_cast<int>(output.size()));
	return ieml_cbor::from_cbor<FileInclude_>(input, anchor_keeper).get_clear_file();
}

TEST(into, null) {
	ieml::Node node{ieml::NullData{}};
	ASSERT_EQ(node, test_into(node));
}

TEST(into, raw) {
	ieml::Node node{ieml::RawData{"hello"}};
	ASSERT_EQ(node, test_into(node));
}

TEST(into, string) {
	ieml::Node node{ieml::StringData{"hello"}};
	ASSERT_EQ(node, test_into(node));
}

TEST(into, list) {
	ieml::Node node{ieml::ListData{ieml::NullData{}, ieml::RawData{"hello"}}};
	ASSERT_EQ(node, test_into(node));
}

TEST(into, map) {
	ieml::Node node{ieml::MapData{{"first", ieml::NullData{}}, {"second", ieml::RawData{"hello"}}}};
	ASSERT_EQ(node, test_into(node));
}

TEST(into, tag) {
	ieml::Node node{ieml::TagData{ieml::NullData{}, "hello"}};
	ASSERT_EQ(node, test_into(node));
}

static std::map<ieml::String, std::vector<uint8_t> > files{};

struct FileInclude {
	static ieml::NodeData include(const ieml::RcPtr<ieml::AnchorKeeper>& anchor_keeper, const ieml::FilePath& file_path) {
		cbor::Input input{files[file_path.string()].data(), static_cast<int>(files[file_path.string()].size())};
		ieml_cbor::BasicParser<FileInclude> parser{input, anchor_keeper, file_path};
		return parser.parse();
	}
};

struct FileGenerate {
	static void generate(const ieml::Node& node, const ieml::FilePath& file_path) {
		cbor::OutputDynamic output{};
		cbor::Encoder encoder{output};
		ieml_cbor::into_cbor(encoder, node);
		files.emplace(file_path.string(), std::vector<uint8_t>{output.data(), output.data() + output.size()});
	}
};

TEST(into, file) {
	auto output_anchor_keeper{ieml::make_rc_ptr<ieml::AnchorKeeper>()};
	output_anchor_keeper->add_to_file("anchor", ieml::NullData{});
	ieml::Node node{ieml::FileData{ieml::NullData{}, ieml::FilePath{"test.cbor"}, output_anchor_keeper}};
	
	ASSERT_EQ(node, (test_into<FileInclude, FileGenerate>(node)));
}

TEST(into, take_anchor) {
	auto output_anchor_keeper{ieml::make_rc_ptr<ieml::AnchorKeeper>()};
	output_anchor_keeper->add("anchor", ieml::RawData{"hello"});
	ieml::Node node{ieml::TakeAnchorData{output_anchor_keeper, "anchor"}};
	
	auto input_anchor_keeper{ieml::make_rc_ptr<ieml::AnchorKeeper>()};
	ASSERT_EQ(node, (test_into(node, input_anchor_keeper)));
	ASSERT_EQ(output_anchor_keeper->get_map(), input_anchor_keeper->get_map());
}

TEST(into, get_anchor) {
	auto output_anchor_keeper{ieml::make_rc_ptr<ieml::AnchorKeeper>()};
	output_anchor_keeper->add("anchor", ieml::RawData{"hello"});
	ieml::Node node{ieml::GetAnchorData{output_anchor_keeper, "anchor"}};
	
	ASSERT_EQ(node, (test_into(node, output_anchor_keeper)));
	ASSERT_EQ(node.get_clear_get_anchor(), *output_anchor_keeper->get("anchor"));
}