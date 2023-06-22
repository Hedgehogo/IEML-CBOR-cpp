#include <gtest/gtest.h>
#include <cbor/cbor.hpp>
#include <IEML_CBOR/from.hpp>
#include <IEML_CBOR/into.hpp>

template<typename FileInclude_ = ieml_cbor::FileInclude, typename FileGenerate_ = ieml_cbor::FileGenerate>
ieml::Node test_into(ieml::Node const& node, ieml::RcPtr<ieml::AnchorKeeper> const& anchorKeeper = ieml::makeRcPtr<ieml::AnchorKeeper>()) {
	cbor::OutputDynamic output;
	cbor::Encoder encoder(output);
	ieml_cbor::intoCBOR<FileGenerate_>(encoder, ieml::FileData{node, {}, ieml::makeRcPtr<ieml::AnchorKeeper>()});
	
	cbor::Input input(output.data(), static_cast<int>(output.size()));
	return ieml_cbor::fromCBOR<FileInclude_>(input, anchorKeeper).getClearFile();
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
	static ieml::NodeData include(const ieml::RcPtr<ieml::AnchorKeeper>& anchorKeeper, const ieml::FilePath& filePath) {
		cbor::Input input{files[filePath.string()].data(), static_cast<int>(files[filePath.string()].size())};
		ieml_cbor::BasicParser<FileInclude> parser{input, anchorKeeper, filePath};
		return parser.parse();
	}
};

struct FileGenerate {
	static void generate(const ieml::Node& node, const ieml::FilePath& filePath) {
		cbor::OutputDynamic output{};
		cbor::Encoder encoder{output};
		ieml_cbor::intoCBOR(encoder, node);
		files.emplace(filePath.string(), std::vector<uint8_t>{output.data(), output.data() + output.size()});
	}
};

TEST(into, file) {
	auto outputAnchorKeeper{ieml::makeRcPtr<ieml::AnchorKeeper>()};
	outputAnchorKeeper->addToFile("anchor", ieml::NullData{});
	ieml::Node node{ieml::FileData{ieml::NullData{}, ieml::FilePath{"test.cbor"}, outputAnchorKeeper}};
	
	ASSERT_EQ(node, (test_into<FileInclude, FileGenerate>(node)));
}

TEST(into, takeAnchor) {
	auto outputAnchorKeeper{ieml::makeRcPtr<ieml::AnchorKeeper>()};
	outputAnchorKeeper->add("anchor", ieml::RawData{"hello"});
	ieml::Node node{ieml::TakeAnchorData{outputAnchorKeeper, "anchor"}};
	
	auto inputAnchorKeeper{ieml::makeRcPtr<ieml::AnchorKeeper>()};
	ASSERT_EQ(node, (test_into(node, inputAnchorKeeper)));
	ASSERT_EQ(outputAnchorKeeper->getMap(), inputAnchorKeeper->getMap());
}

TEST(into, getAnchor) {
	auto outputAnchorKeeper{ieml::makeRcPtr<ieml::AnchorKeeper>()};
	outputAnchorKeeper->add("anchor", ieml::RawData{"hello"});
	ieml::Node node{ieml::GetAnchorData{outputAnchorKeeper, "anchor"}};
	
	ASSERT_EQ(node, (test_into(node, outputAnchorKeeper)));
	ASSERT_EQ(node.getClearGetAnchor(), *outputAnchorKeeper->get("anchor"));
}