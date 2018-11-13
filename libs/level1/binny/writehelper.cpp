#include "core/core.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <cctype>
#include "writehelper.h"
#include "bundle.h" 		// for header flags

namespace Binny {

WriteHelper::WriteHelper() : o(ostr)
{
	// system labels
	reserve_label("stringTable"s);
	reserve_label("stringTableEnd"s);
	reserve_label("chunks"s);
	reserve_label("chunksEnd"s);

	reserve_label("beginEnd"s);
}

void WriteHelper::set_variable(std::string const& name, int64_t value, bool pass0, std::string const comment_, bool noCommentEndStatement_)
{
	set_variable_to_expression(name, std::to_string(value), pass0, comment_, noCommentEndStatement_);
}

void WriteHelper::set_variable_to_expression(std::string const& name, std::string const& exp, bool pass0, std::string const comment_, bool noCommentEndStatement_)
{
	variables.insert(name);

	// pass 0 variables output the last set in pass 0, so work for counters
	if (pass0)
	{
		o << "*VAR_" << name << "* " << exp;
	}
	else
	{
		o << "VAR_" << name << "= " << exp;
	}
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::set_constant(std::string const& name, int64_t value, std::string const comment_, bool noCommentEndStatement_)
{
	set_constant_to_expression(name, std::to_string(value), comment_, noCommentEndStatement_);
}

void WriteHelper::set_constant_to_expression(std::string const& name, std::string const& exp, std::string const comment_, bool noCommentEndStatement_)
{
	assert(constants.find(name) == constants.end());
	constants.insert(name);
	o << "*CONST_" << name << "* " << exp;
	comment(comment_, noCommentEndStatement_);
}
std::string WriteHelper::get_variable(std::string const& name)
{
	assert(variables.find(name) != variables.end());
	return " VAR_" + name;
}

std::string WriteHelper::get_constant(std::string const& name)
{
	assert(constants.find(name) != constants.end());
	return " CONST_" + name;
}

void WriteHelper::increment_variable(std::string const& str_, std::string const comment_, bool noCommentEndStatement_)
{
	std::string var = get_variable(str_);
	set_variable_to_expression(str_, var + "+ 1");
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::add_enum(std::string const& name)
{
	assert(enums.find(name) == enums.end());
	enums[name] = {};
}

void WriteHelper::add_enum_value(std::string const& name, std::string const& value_name, uint64_t value, std::string const comment_, bool noCommentEndStatement_)
{
	assert(enums.find(name) != enums.end());
	auto& e = enums[name];
	assert(e.find(value_name) == e.end());
	e[value_name] = value;
	set_constant(name + "_" + value_name, value,comment_, noCommentEndStatement_);
}

std::string WriteHelper::get_enum_value(std::string const& name, std::string const& value_name)
{
	assert(enums.find(name) != enums.end());
	auto& e = enums[name];
	assert(e.find(value_name) != e.end());

	return get_constant(name + "_" + value_name);
}

void WriteHelper::write_enum(std::string const& name, std::string const& value_name, std::string const comment_, bool noCommentEndStatement_)
{
	o << get_enum_value(name, value_name);
	comment(comment_, noCommentEndStatement_);

}

void WriteHelper::write_flags(std::string const& name, uint64_t flags, std::string const comment_, bool noCommentEndStatement_)
{
	assert(enums.find(name) != enums.end());
	auto& e = enums[name];
	o << "0";
	for(auto[ename, val] : e)
	{
		if(flags & val)
		{
			o << " | " << get_enum_value(name, ename);
		}
	}
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::write_chunk_header(uint16_t majorVersion_, uint16_t minorVersion_)
{
	// write header
	comment("---------------------------------------------"s);
	comment("Chunk"s);
	comment("---------------------------------------------"s);
	o << ".type u" << std::to_string(addressLen) << std::endl;

	write_label("chunk_begin"s, true);
	set_string_table_base("chunk_begin"s);
	reserve_label("fixups"s);
	reserve_label("data"s, true);
	reserve_label("fixupsEnd"s);
	reserve_label("dataEnd"s);

	size_of_block("fixups"s);
	size_of_block("data"s);
	use_label("fixups"s, "chunk_begin"s, false, false );
	use_label("data"s, "chunk_begin"s, false, false );
	write_as<uint16_t>(majorVersion_, "Major Version"s);
	write_as<uint16_t>(minorVersion_, "Minor Version"s);
	align();
	set_default_type<uint32_t>();
	set_string_table_base("data"s);
}

void WriteHelper::write_bundle_header(uint64_t const userData_)
{
	// write header
	comment("---------------------------------------------"s);
	comment("Bundle"s);
	comment("---------------------------------------------"s);
	set_default_type<uint32_t>();

	add_enum("HeaderFlag");
	add_enum_value("HeaderFlag", "64Bit"s, Bundle::HeaderFlag_64Bit);
	add_enum_value("HeaderFlag", "32Bit"s, Bundle::HeaderFlag_32Bit);
	set_variable("DirEntryCount"s, 0, true);

	// magic
	write("BUND"_bundle_id, "magic"s);

	// flags
	uint32_t flags = (addressLen == 32) ? Bundle::HeaderFlag_32Bit : Bundle::HeaderFlag_64Bit;
	write_flags("HeaderFlag"s, flags);

	// version
	write_as<uint16_t>(Bundle::majorVersion, Bundle::minorVersion, "major, minor version"s);

	// micro offsets
	write_expression_as<uint16_t>("stringTable - beginEnd"s, "strings micro offset"s);
	write_expression_as<uint16_t>("chunks - stringTableEnd"s, "chunks micro offset"s);

	// 64 bit user data
	write_as<uint64_t>(userData_);

	// string table size
	size_of_block("stringTable"s);

	// directory entry count
	write_expression_as<uint32_t>(get_variable("DirEntryCount"), "Directory entry count"s);

}

void WriteHelper::comment(std::string const& comment, bool noCommentEndStatement) const
{
	if (!comment.empty())
	{
		o << "// " << comment << std::endl;
	}
	else if(noCommentEndStatement)
	{
		o << std::endl;
	}
}

void WriteHelper::allow_nan(bool yesno) const
{
	o << ".allownan " << (yesno ? "1" : "0") << std::endl;
}

void WriteHelper::allow_infinity(bool yesno) const
{
	o << ".allowinfinity " << (yesno ? "1" : "0") << std::endl;
}

void WriteHelper::align(int i) const
{
	o << ".align" << std::to_string(i) << std::endl;
}

void WriteHelper::align() const
{
	align(addressLen / 8);
}

void WriteHelper::reserve_label(std::string const& name, bool makeDefault)
{
	std::string label = name;
	assert(labels.find(label) == labels.end());
	labels.insert(label);

	if (makeDefault)
	{
		defaultBlock = label;
	}
}

void WriteHelper::write_label(std::string const& name, bool reserve, std::string const comment_, bool noCommentEndStatement_)
{
	if (reserve)
	{
		reserve_label(name);
	}

	assert(labels.find(name) != labels.end());
	align();
	o << name << ":";
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::use_label(std::string const& name, std::string baseBlock, bool reserve, bool addFixup,  std::string const comment_, bool noCommentEndStatement_)
{
	if (reserve)
	{
		reserve_label(name);
	}
	assert(labels.find(name) != labels.end());

	if (addFixup)
	{
		std::string fixupLabel = "FIXUP_"s + std::to_string(fixups.size());
		fixups.push_back(fixupLabel);
		write_label(fixupLabel, true);
	}

	if (baseBlock.empty())
	{
		baseBlock = defaultBlock;
	}
	assert(name != baseBlock);

	o << ".fixup " << name << " - " << baseBlock;
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::set_string_table_base(std::string const& label)
{
	stringTableBase = label;
}

void WriteHelper::add_string(std::string_view str_)
{
	std::string str = std::string(str_);
	std::string stringLabel = add_string_to_table(str);
	use_label(stringLabel, stringTableBase, false, true, str);
}

std::string WriteHelper::add_string_to_table(std::string const& str)
{
	std::string stringLabel;
	// dedup strings
	if (reverseStringTable.find(str) == reverseStringTable.end())
	{
		stringLabel = "STR_"s + nameToLabel(str);

		labelToStringTable[stringLabel] = str;
		reverseStringTable[str] = stringLabel;

		reserve_label(stringLabel);
	}
	else
	{
		stringLabel = reverseStringTable[str];
	}
	return stringLabel;
}

std::string WriteHelper::nameToLabel(std::string const& name)
{
	std::string clean = name;
	for(auto& ch : clean)
	{
		if(!std::isdigit(ch) && !std::isalpha(ch))
		{
			ch = '_';
		}
	}

	return clean;
}

void WriteHelper::merge_string_table(WriteHelper& other)
{
	for (auto[label, str] : other.labelToStringTable)
	{
		std::string newLabel = add_string_to_table(str);
		assert(newLabel == label);
		assert(labelToStringTable.find(label) != labelToStringTable.end());
	}
}

void WriteHelper::finish_string_table()
{
	align();
	write_label("stringTable"s);

	for (auto[label, str] : labelToStringTable)
	{
		write_label(label);
		o << "\"" << str << "\"" << ", 0" << std::endl;
	}

	write_label("stringTableEnd"s);
}

void WriteHelper::size_of_block(std::string const& name, std::string const comment_, bool noCommentEndStatement_)
{
	std::string const nameend = name + "End"s;

	assert(labels.find(name) != labels.end());
	assert(labels.find(nameend) != labels.end());
	o << nameend << " - " << name;

	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::set_address_length(int bits)
{
	o << ".addresslen " << bits; comment("Using " + std::to_string(bits) + " bits for addresses");
	addressLen = bits;
}

void WriteHelper::write_null_ptr(std::string const comment_, bool noCommentEndStatement_)
{
	write_address_type(); o << "0"; 
	comment(comment_, noCommentEndStatement_);
}
void WriteHelper::write_address_type()
{
	o << "(u" << std::to_string(addressLen) << ")";
}

void WriteHelper::write_byte_array(std::vector<uint8_t> const& barray)
{
	write_byte_array(barray.data(), barray.size());
}

void WriteHelper::write_byte_array(uint8_t const* bytes_, size_t size_)
{
	set_default_type<uint8_t>();
	if (size_ == 0)
	{
		o << "0" << std::endl;
	}
	else
	{
		for (size_t i = 0; i < size_ - 1; i++)
		{
			o << std::to_string(bytes_[i]);
			if ((i % 80) == 79)
			{
				o << std::endl;
			}
			else
			{
				o << ", ";
			}

		}
		// write last byte without ,
		o << std::to_string(bytes_[size_ - 1]) << std::endl;
	}

	set_default_type<uint32_t>();
}

} // end namespace