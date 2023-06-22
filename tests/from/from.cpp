#include <gtest/gtest.h>
#include <cbor/cbor.hpp>
#include <IEML_CBOR/from.hpp>

TEST(from, null) {
	cbor::OutputDynamic output;
	cbor::Encoder encoder(output);
	encoder.write_array(2);
	encoder.write_int(0);
	encoder.write_null();
	
	cbor::Input input(output.data(), static_cast<int>(output.size()));
	const auto null{ieml_cbor::fromCBOR(input)};
	ASSERT_EQ(null.getClearFile().getType(), ieml::NodeType::Null);
}

TEST(from, raw) {
	cbor::OutputDynamic output;
	cbor::Encoder encoder(output);
	encoder.write_array(2);
	encoder.write_int(1);
	encoder.write_string("hello");
	
	cbor::Input input(output.data(), static_cast<int>(output.size()));
	const auto raw{ieml_cbor::fromCBOR(input)};
	ASSERT_EQ(raw.getClearFile().getType(), ieml::NodeType::Raw);
	ASSERT_EQ(raw.getClearFile().getRaw().except().str, "hello");
}

TEST(from, string) {
	cbor::OutputDynamic output;
	cbor::Encoder encoder(output);
	encoder.write_array(2);
	encoder.write_int(2);
	encoder.write_string("hello");
	
	cbor::Input input(output.data(), static_cast<int>(output.size()));
	const auto string{ieml_cbor::fromCBOR(input)};
	ASSERT_EQ(string.getClearFile().getType(), ieml::NodeType::String);
	ASSERT_EQ(string.getClearFile().getString().except(), "hello");
}

TEST(from, list) {
	cbor::OutputDynamic output;
	cbor::Encoder encoder(output);
	encoder.write_array(2);
	encoder.write_int(3);
	encoder.write_array(2);
	{
		encoder.write_array(2);
		encoder.write_int(0);
		encoder.write_null();
	}
	{
		encoder.write_array(2);
		encoder.write_int(2);
		encoder.write_string("hello");
	}
	
	cbor::Input input(output.data(), static_cast<int>(output.size()));
	const auto list{ieml_cbor::fromCBOR(input)};
	ASSERT_EQ(list.getClearFile().getType(), ieml::NodeType::List);
	ASSERT_EQ(list.getListSize().except(), 2);
	ASSERT_EQ(list.at(0).except().getType(), ieml::NodeType::Null);
	ASSERT_EQ(list.at(1).except().getType(), ieml::NodeType::String);
	ASSERT_EQ(list.at(1).except().getString().except(), "hello");
}

TEST(from, map) {
	cbor::OutputDynamic output;
	cbor::Encoder encoder(output);
	encoder.write_array(2);
	encoder.write_int(4);
	encoder.write_map(2);
	{
		encoder.write_string("first");
		encoder.write_array(2);
		encoder.write_int(0);
		encoder.write_null();
	}
	{
		encoder.write_string("second");
		encoder.write_array(2);
		encoder.write_int(2);
		encoder.write_string("hello");
	}
	
	cbor::Input input(output.data(), static_cast<int>(output.size()));
	const auto map{ieml_cbor::fromCBOR(input)};
	ASSERT_EQ(map.getClearFile().getType(), ieml::NodeType::Map);
	ASSERT_EQ(map.getMapSize().except(), 2);
	ASSERT_EQ(map.getMap().except().at("first").getType(), ieml::NodeType::Null);
	ASSERT_EQ(map.getMap().except().at("second").getType(), ieml::NodeType::String);
	ASSERT_EQ(map.getMap().except().at("second").getString().except(), "hello");
}

TEST(from, tag) {
	cbor::OutputDynamic output;
	cbor::Encoder encoder(output);
	encoder.write_array(2);
	encoder.write_int(5);
	encoder.write_array(2);
	encoder.write_string("tag");
	{
		encoder.write_array(2);
		encoder.write_int(2);
		encoder.write_string("hello");
	}
	
	cbor::Input input(output.data(), static_cast<int>(output.size()));
	const auto tag{ieml_cbor::fromCBOR(input)};
	ASSERT_EQ(tag.getClearFile().getType(), ieml::NodeType::Tag);
	ASSERT_EQ(tag.getTag().except(), "tag");
	ASSERT_EQ(tag.getClearFile().getClearTag().getType(), ieml::NodeType::String);
	ASSERT_EQ(tag.getString().except(), "hello");
}

static std::map<ieml::String, std::vector<uint8_t> > files{
	std::make_pair("test.cbor", []{
		cbor::OutputDynamic output;
		cbor::Encoder encoder(output);
		encoder.write_array(2);
		encoder.write_int(0);
		encoder.write_null();
		return std::vector<uint8_t>{output.data(), output.data() + output.size()};
	}())
};

struct FileInclude {
	static ieml::NodeData include(const ieml::RcPtr<ieml::AnchorKeeper>& anchorKeeper, const ieml::FilePath& filePath) {
		cbor::Input input{files[filePath.string()].data(), static_cast<int>(files[filePath.string()].size())};
		ieml_cbor::BasicParser<FileInclude> parser{input, anchorKeeper, filePath};
		return parser.parse();
	}
};

TEST(from, file) {
	cbor::OutputDynamic output;
	cbor::Encoder encoder(output);
	encoder.write_array(2);
	encoder.write_int(6);
	encoder.write_array(2);
	encoder.write_string("test.cbor");
	{
		encoder.write_map(1);
		encoder.write_string("anchor-name");
		encoder.write_array(2);
		encoder.write_int(2);
		encoder.write_string("hello");
	}
	
	cbor::Input input(output.data(), static_cast<int>(output.size()));
	const auto file{ieml_cbor::fromCBOR<::FileInclude>(input)};
	ASSERT_EQ(file.getClearFile().getType(), ieml::NodeType::File);
	ASSERT_EQ(file.getClearFile().getFilePath().except(), "test.cbor");
	ASSERT_EQ(file.getClearFile().getClearFile().getType(), ieml::NodeType::Null);
	ASSERT_EQ(file.getClearFile().getFileAnchorKeeper().except().getFileMap().at("anchor-name").getType(), ieml::NodeType::String);
	ASSERT_EQ(file.getClearFile().getFileAnchorKeeper().except().getFileMap().at("anchor-name").getString().except(), "hello");
}

TEST(from, takeAnchor) {
	cbor::OutputDynamic output;
	cbor::Encoder encoder(output);
	encoder.write_array(2);
	encoder.write_int(7);
	encoder.write_array(2);
	encoder.write_string("take-name");
	{
		encoder.write_array(2);
		encoder.write_int(2);
		encoder.write_string("hello");
	}
	
	cbor::Input input(output.data(), static_cast<int>(output.size()));
	const auto map{ieml_cbor::fromCBOR(input)};
	ASSERT_EQ(map.getClearFile().getType(), ieml::NodeType::TakeAnchor);
	ASSERT_EQ(map.getTakeAnchorName().except(), "take-name");
	ASSERT_EQ(map.getClearFile().getClearTakeAnchor().getType(), ieml::NodeType::String);
	ASSERT_EQ(map.getString().except(), "hello");
}

TEST(from, getAnchor) {
	cbor::OutputDynamic output;
	cbor::Encoder encoder(output);
	encoder.write_array(2);
	encoder.write_int(8);
	encoder.write_string("get-name");
	
	cbor::Input input(output.data(), static_cast<int>(output.size()));
	const auto map{ieml_cbor::fromCBOR(input)};
	ASSERT_EQ(map.getClearFile().getType(), ieml::NodeType::GetAnchor);
	ASSERT_EQ(map.getGetAnchorName().except(), "get-name");
}