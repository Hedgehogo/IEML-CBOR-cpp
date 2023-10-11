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
	const auto null{ieml_cbor::from_cbor(input)};
	ASSERT_EQ(null.get_clear_file().get_type(), ieml::NodeType::Null);
}

TEST(from, raw) {
	cbor::OutputDynamic output;
	cbor::Encoder encoder(output);
	encoder.write_array(2);
	encoder.write_int(1);
	encoder.write_string("hello");
	
	cbor::Input input(output.data(), static_cast<int>(output.size()));
	const auto raw{ieml_cbor::from_cbor(input)};
	ASSERT_EQ(raw.get_clear_file().get_type(), ieml::NodeType::Raw);
	ASSERT_EQ(raw.get_clear_file().get_raw().except().str, "hello");
}

TEST(from, string) {
	cbor::OutputDynamic output;
	cbor::Encoder encoder(output);
	encoder.write_array(2);
	encoder.write_int(2);
	encoder.write_string("hello");
	
	cbor::Input input(output.data(), static_cast<int>(output.size()));
	const auto string{ieml_cbor::from_cbor(input)};
	ASSERT_EQ(string.get_clear_file().get_type(), ieml::NodeType::String);
	ASSERT_EQ(string.get_clear_file().get_string().except(), "hello");
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
	const auto list{ieml_cbor::from_cbor(input)};
	ASSERT_EQ(list.get_clear_file().get_type(), ieml::NodeType::List);
	ASSERT_EQ(list.get_list_size().except(), 2);
	ASSERT_EQ(list.at(0).except().get_type(), ieml::NodeType::Null);
	ASSERT_EQ(list.at(1).except().get_type(), ieml::NodeType::String);
	ASSERT_EQ(list.at(1).except().get_string().except(), "hello");
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
	const auto map{ieml_cbor::from_cbor(input)};
	ASSERT_EQ(map.get_clear_file().get_type(), ieml::NodeType::Map);
	ASSERT_EQ(map.get_map_size().except(), 2);
	ASSERT_EQ(map.get_map().except().at("first").get_type(), ieml::NodeType::Null);
	ASSERT_EQ(map.get_map().except().at("second").get_type(), ieml::NodeType::String);
	ASSERT_EQ(map.get_map().except().at("second").get_string().except(), "hello");
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
	const auto tag{ieml_cbor::from_cbor(input)};
	ASSERT_EQ(tag.get_clear_file().get_type(), ieml::NodeType::Tag);
	ASSERT_EQ(tag.get_tag().except(), "tag");
	ASSERT_EQ(tag.get_clear_file().get_clear_tag().get_type(), ieml::NodeType::String);
	ASSERT_EQ(tag.get_string().except(), "hello");
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
	static ieml::NodeData include(const ieml::RcPtr<ieml::AnchorKeeper>& anchor_keeper, const ieml::FilePath& file_path) {
		cbor::Input input{files[file_path.string()].data(), static_cast<int>(files[file_path.string()].size())};
		ieml_cbor::BasicParser<FileInclude> parser{input, anchor_keeper, file_path};
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
	const auto file{ieml_cbor::from_cbor<::FileInclude>(input)};
	ASSERT_EQ(file.get_clear_file().get_type(), ieml::NodeType::File);
	ASSERT_EQ(file.get_clear_file().get_file_path().except(), "test.cbor");
	ASSERT_EQ(file.get_clear_file().get_clear_file().get_type(), ieml::NodeType::Null);
	ASSERT_EQ(file.get_clear_file().get_file_anchor_keeper().except().get_file_map().at("anchor-name").get_type(), ieml::NodeType::String);
	ASSERT_EQ(file.get_clear_file().get_file_anchor_keeper().except().get_file_map().at("anchor-name").get_string().except(), "hello");
}

TEST(from, take_anchor) {
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
	const auto map{ieml_cbor::from_cbor(input)};
	ASSERT_EQ(map.get_clear_file().get_type(), ieml::NodeType::TakeAnchor);
	ASSERT_EQ(map.get_take_anchor_name().except(), "take-name");
	ASSERT_EQ(map.get_clear_file().get_clear_take_anchor().get_type(), ieml::NodeType::String);
	ASSERT_EQ(map.get_string().except(), "hello");
}

TEST(from, get_anchor) {
	cbor::OutputDynamic output;
	cbor::Encoder encoder(output);
	encoder.write_array(2);
	encoder.write_int(8);
	encoder.write_string("get-name");
	
	cbor::Input input(output.data(), static_cast<int>(output.size()));
	const auto map{ieml_cbor::from_cbor(input)};
	ASSERT_EQ(map.get_clear_file().get_type(), ieml::NodeType::GetAnchor);
	ASSERT_EQ(map.get_get_anchor_name().except(), "get-name");
}