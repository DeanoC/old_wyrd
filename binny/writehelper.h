#pragma once
#ifndef BINNY_WRITE_HELPER_H
#define BINNY_WRITE_HELPER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

namespace Binny {
using namespace std::string_literals;

class WriteHelper
{
public:
	friend class BundleWriter;

	WriteHelper();

	// defaults
	void default_type(std::string const& type) const;
	void allow_nan(bool yesno) const;
	void allow_infinity(bool yesno) const;
	void set_address_length(int bits);

	// alignment function
	void align(int i) const;
	void align() const;

	// label functions
	void reserve_label(std::string const& name, bool makeDefault = false);
	void write_label(std::string const& name, bool reserve = false, std::string const comment_ = ""s, bool noCommentEndStatement_ = true);
	void use_label(std::string const& name, std::string baseBlock = ""s, bool reserve = false, bool addFixup = true,  std::string const comment_ = ""s, bool noCommentEndStatement_ = true);

	// constants
	void set_constant(std::string const& name, int64_t value, std::string const comment_ = ""s, bool noCommentEndStatement_ = true);
	std::string get_constant(std::string const& name);
	void set_constant_to_expression(std::string const& name, std::string const& exp, std::string const comment_ = ""s, bool noCommentEndStatement_ = true);

	// variables
	void set_variable(std::string const& name, int64_t value, bool pass0 = false, std::string const comment_ = ""s, bool noCommentEndStatement_ = true);
	void set_variable_to_expression(std::string const& name, std::string const& exp, bool pass0 = false, std::string const comment_ = ""s, bool noCommentEndStatement_ = true);
	std::string get_variable(std::string const& name);
	void increment_variable(std::string const& str_, std::string const comment_ = ""s, bool noCommentEndStatement_ = true);

	// enum and flags functions
	void add_enum(std::string const& name);
	void add_enum_value(std::string const& enum_name, std::string const& value_name, uint64_t value, std::string const comment_ = ""s, bool noCommentEndStatement_ = true);
	std::string get_enum_value(std::string const& name, std::string const& value_name);
	void write_enum(std::string const& name, std::string const& value_name, std::string const comment_ = ""s, bool noCommentEndStatement_ = true);
	void write_flags(std::string const& name, uint64_t flags, std::string const comment_ = ""s, bool noCommentEndStatement_ = true);

	// string table functiona
	void add_string(std::string const& str); 	///< adds it to the table and outputs a fixup
	std::string add_string_to_table(std::string const& str);
	void set_string_table_base(std::string const& label);

	// expression functions
	void write_expression(std::string const& str_, std::string const comment_ = ""s, bool noCommentEndStatement_ = true) { o << str_; comment(comment_, noCommentEndStatement_); }
	template<typename type>
	void write_expression_as(std::string const& str_, std::string const comment_ = ""s, bool noCommentEndStatement_ = true)
	{
		o << "(" << type_to_string<type>() << ") " << str_;
		comment(comment_, noCommentEndStatement_);
	}

	// misc functions
	void size_of_block(std::string const& name, std::string const comment = ""s, bool noCommentEndStatement_ = true);
	void comment(std::string const& comment, bool noCommentEndStatement_ = true) const;

	// writing functions
	void write_null_ptr(std::string const comment = ""s, bool noCommentEndStatement_ = true); // outputs an address size 0 (without fixup of course!)
	void write_address_type(); ///< ouputs a address type prefix
	void write_byte_array(std::vector<uint8_t> const& barray); ///< writes a byte array

	void write_chunk_header(uint16_t majorVersion_, uint16_t minorVersion_); ///< writes out a chunk header for bundles
	void write_bundle_header(uint64_t const userData_); ///< writes outa bundle header for bundles

	// template single element write with optional comment
	template<typename T>
	void write(T i_, std::string const comment_ = ""s) { o << std::to_string(i_); comment(comment_); }

	// template 2 element write with optional comment
	template<typename T>
	void write(T i0_, T i1_, std::string const comment_ = ""s) { 
		o << std::to_string(i0_) << ", "  << std::to_string(i1_);
		comment(comment_); 
	}

	// template 3 element write with optional comment
	template<typename T>
	void write(T i0_, T i1_, T i2_, std::string const comment_ = ""s) {
		o << std::to_string(i0_) << ", " << std::to_string(i1_) << ", " << std::to_string(i2_);
		comment(comment_);
	}

	// template 4 element write with optional comment
	template<typename T>
	void write(T i0_, T i1_, T i2_, T i3_, std::string const comment_ = ""s) {
		o	<< std::to_string(i0_) << ", " << std::to_string(i1_) << ", "
			<< std::to_string(i2_) << ", " << std::to_string(i3_);
		comment(comment_);
	}

	// returns the binify type string for supported types (except strings)
	template<typename T> std::string type_to_string() const {
		using namespace std::string_literals;
		if (std::is_signed<T>())
		{
			if (typeid(typename T) == typeid(double)) { return "Double"s; }
			if (typeid(T) == typeid(float)) { return "Float"s; }

			switch (sizeof(T)) {
			case 1: return "s8"s;
			case 2: return "s16"s;
			case 4: return "s32"s;
			case 8: return "s64"s;
			default: assert(sizeof(T) == -1);
			}
		} else {
			switch (sizeof(T)) {
			case 1: return "u8"s;
			case 2: return "u16"s;
			case 4: return "u32"s;
			case 8: return "u64"s;
			default: assert(sizeof(T) == -1);
			}
		}
		return "unknown"s;
	}

	// template single element write as type with optional comment
	template<typename type, typename T>
	void write_as(T i_, std::string const comment_ = ""s) {
		o << "(" << type_to_string<type>() << ") " << std::to_string(i_);
		comment(comment_);
	}

	template<typename type, typename T>
	void write_as(T i0_, T i1_, std::string const comment_ = ""s)
	{
		o << "(" << type_to_string<type>() << ") " << std::to_string(i0_) << ", ";
		o << " (" << type_to_string<type>() << ") " << std::to_string(i1_);
		comment(comment_);
	}

	template<typename type, typename T>
	void write_as(T i0_, T i1_, T i2_, std::string const comment_ = ""s)
	{
		o << "(" << type_to_string<type>() << ") " << std::to_string(i0_) << ", ";
		o << " (" << type_to_string<type>() << ") " << std::to_string(i1_) << ", ";
		o << " (" << type_to_string<type>() << ") " << std::to_string(i2_);

		comment(comment_);
	}

	template<typename type, typename T>
	void write_as(T i0_, T i1_, T i2_, T i3_, std::string const comment_ = ""s)
	{
		o << "(" << type_to_string<type>() << ") " << std::to_string(i0_) << ", ";
		o << " (" << type_to_string<type>() << ") " << std::to_string(i1_) << ", ";
		o << " (" << type_to_string<type>() << ") " << std::to_string(i2_) << ", ";
		o << " (" << type_to_string<type>() << ") " << std::to_string(i3_);
		comment(comment_);
	}

	template<typename T>
	void write_size(T i_, std::string const comment_ = ""s)
	{
		write_address_type();
		o << std::to_string(i_);
		comment(comment_);
	}

	std::ostream& o;
	std::ostringstream ostr;
private: 
	std::string stringTableBase = "stringTable"s;

	void merge_string_table(WriteHelper& other);
	void finish_string_table();
	void clear_string_table();

	std::string nameToLabel(std::string const& name);
	std::unordered_map<std::string, std::string> labelToStringTable;
	std::unordered_map<std::string, std::string> reverseStringTable;

	std::vector<std::string> fixups;
	std::unordered_set<std::string> labels;
	std::unordered_set<std::string> variables;
	std::unordered_set<std::string> constants;
	std::unordered_map<std::string, std::unordered_map<std::string, uint64_t>> enums;

	std::string defaultBlock;
	int addressLen = 64;

};

} // end namespace

#endif //BINNY_WRITE_HELPER_H