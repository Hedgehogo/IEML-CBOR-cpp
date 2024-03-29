#include <gtest/gtest.h>
#include <cbor/cbor.hpp>
#include <IEML_CBOR/from.hpp>

TEST(from, null) {
	auto output{cbor::OutputDynamic{}};
	auto encoder{cbor::Encoder{output}};
	encoder.write_array(2);
	encoder.write_int(0);
	encoder.write_null();
	
	auto input{cbor::Input{output.data(), static_cast<int>(output.size())}};
	auto const null{ieml_cbor::from_cbor(input)};
	ASSERT_EQ(null.get_clear_file().get_type(), ieml::NodeType::Null);
}

TEST(from, raw) {
	auto output{cbor::OutputDynamic{}};
	auto encoder{cbor::Encoder{output}};
	encoder.write_array(2);
	encoder.write_int(1);
	encoder.write_string("hello");
	
	auto input{cbor::Input{output.data(), static_cast<int>(output.size())}};
	auto const raw{ieml_cbor::from_cbor(input)};
	ASSERT_EQ(raw.get_clear_file().get_type(), ieml::NodeType::Raw);
	ASSERT_EQ(raw.get_clear_file().get_raw().except().str, "hello");
}

TEST(from, string) {
	auto output{cbor::OutputDynamic{}};
	auto encoder{cbor::Encoder{output}};
	encoder.write_array(2);
	encoder.write_int(2);
	encoder.write_string("hello");
	
	auto input{cbor::Input{output.data(), static_cast<int>(output.size())}};
	auto const string{ieml_cbor::from_cbor(input)};
	ASSERT_EQ(string.get_clear_file().get_type(), ieml::NodeType::String);
	ASSERT_EQ(string.get_clear_file().get_string().except(), "hello");
}

TEST(from, list) {
	auto output{cbor::OutputDynamic{}};
	auto encoder{cbor::Encoder{output}};
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
	
	auto input{cbor::Input{output.data(), static_cast<int>(output.size())}};
	auto const list{ieml_cbor::from_cbor(input)};
	ASSERT_EQ(list.get_clear_file().get_type(), ieml::NodeType::List);
	ASSERT_EQ(list.get_list_size().except(), 2);
	ASSERT_EQ(list.at(0).except().get_type(), ieml::NodeType::Null);
	ASSERT_EQ(list.at(1).except().get_type(), ieml::NodeType::String);
	ASSERT_EQ(list.at(1).except().get_string().except(), "hello");
}

TEST(from, map) {
	auto output{cbor::OutputDynamic{}};
	auto encoder{cbor::Encoder{output}};
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
	
	auto input{cbor::Input{output.data(), static_cast<int>(output.size())}};
	auto const map{ieml_cbor::from_cbor(input)};
	ASSERT_EQ(map.get_clear_file().get_type(), ieml::NodeType::Map);
	ASSERT_EQ(map.get_map_size().except(), 2);
	ASSERT_EQ(map.get_map().except().at("first").get_type(), ieml::NodeType::Null);
	ASSERT_EQ(map.get_map().except().at("second").get_type(), ieml::NodeType::String);
	ASSERT_EQ(map.get_map().except().at("second").get_string().except(), "hello");
}

TEST(from, tag) {
	auto output{cbor::OutputDynamic{}};
	auto encoder{cbor::Encoder{output}};
	encoder.write_array(2);
	encoder.write_int(5);
	encoder.write_array(2);
	encoder.write_string("tag");
	{
		encoder.write_array(2);
		encoder.write_int(2);
		encoder.write_string("hello");
	}
	
	auto input{cbor::Input{output.data(), static_cast<int>(output.size())}};
	auto const tag{ieml_cbor::from_cbor(input)};
	ASSERT_EQ(tag.get_clear_file().get_type(), ieml::NodeType::Tag);
	ASSERT_EQ(tag.get_tag().except(), "tag");
	ASSERT_EQ(tag.get_clear_file().get_clear_tag().get_type(), ieml::NodeType::String);
	ASSERT_EQ(tag.get_string().except(), "hello");
}

static std::map<ieml::String, std::vector<uint8_t> > files{
	std::make_pair("test.cbor", []{
		auto output{cbor::OutputDynamic{}};
		auto encoder{cbor::Encoder{output}};
		encoder.write_array(2);
		encoder.write_int(0);
		encoder.write_null();
		return std::vector<uint8_t>{output.data(), output.data() + output.size()};
	}())
};

struct FileInclude {
	static ieml::NodeData include(ieml::RcPtr<ieml::AnchorKeeper> const& anchor_keeper, ieml::FilePath const& file_path) {
		auto input{cbor::Input{files[file_path.string()].data(), static_cast<int>(files[file_path.string()].size())}};
		auto parser{ieml_cbor::BasicParser<FileInclude>{input, anchor_keeper, file_path}};
		return parser.parse();
	}
};

TEST(from, file) {
	auto output{cbor::OutputDynamic{}};
	auto encoder{cbor::Encoder{output}};
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
	
	auto input{cbor::Input{output.data(), static_cast<int>(output.size())}};
	auto const file{ieml_cbor::from_cbor<::FileInclude>(input)};
	ASSERT_EQ(file.get_clear_file().get_type(), ieml::NodeType::File);
	ASSERT_EQ(file.get_clear_file().get_file_path().except(), "test.cbor");
	ASSERT_EQ(file.get_clear_file().get_clear_file().get_type(), ieml::NodeType::Null);
	ASSERT_EQ(file.get_clear_file().get_file_anchor_keeper().except().get_file_map().at("anchor-name").get_type(), ieml::NodeType::String);
	ASSERT_EQ(file.get_clear_file().get_file_anchor_keeper().except().get_file_map().at("anchor-name").get_string().except(), "hello");
}

TEST(from, take_anchor) {
	auto output{cbor::OutputDynamic{}};
	auto encoder{cbor::Encoder{output}};
	encoder.write_array(2);
	encoder.write_int(7);
	encoder.write_array(2);
	encoder.write_string("take-name");
	{
		encoder.write_array(2);
		encoder.write_int(2);
		encoder.write_string("hello");
	}
	
	auto input{cbor::Input{output.data(), static_cast<int>(output.size())}};
	auto const map{ieml_cbor::from_cbor(input)};
	ASSERT_EQ(map.get_clear_file().get_type(), ieml::NodeType::TakeAnchor);
	ASSERT_EQ(map.get_take_anchor_name().except(), "take-name");
	ASSERT_EQ(map.get_clear_file().get_clear_take_anchor().get_type(), ieml::NodeType::String);
	ASSERT_EQ(map.get_string().except(), "hello");
}

TEST(from, get_anchor) {
	auto output{cbor::OutputDynamic{}};
	auto encoder{cbor::Encoder{output}};
	encoder.write_array(2);
	encoder.write_int(8);
	encoder.write_string("get-name");
	
	auto input{cbor::Input{output.data(), static_cast<int>(output.size())}};
	auto const map{ieml_cbor::from_cbor(input)};
	ASSERT_EQ(map.get_clear_file().get_type(), ieml::NodeType::GetAnchor);
	ASSERT_EQ(map.get_get_anchor_name().except(), "get-name");
}