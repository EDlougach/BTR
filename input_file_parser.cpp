#define _CRT_SECURE_NO_DEPRECATE
#include <cstdio>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cfloat>
#include <string>
#include <cerrno>
#include <map>
#include <cassert>
using namespace std;

struct COLUMN_INFORMATION {
	int			column_id;
	bool		regular, has_name;
	double		min_value, max_value;
	int			number_of_elements;
	double		step; // not valid if column data is not regular
	string		name; // supposed column name, can be invalid
};
typedef vector<double> COLUMN;

class FileParser {
private:
	map<string, int>			map_name_id;
	vector<COLUMN>				columns_data;
	vector<COLUMN_INFORMATION>	columns_information;
	static COLUMN_INFORMATION make_column_info(const vector<double> &v);
public:
	inline FileParser(const char *file_name = NULL) // initializing
	{
		if (file_name)
			init_from_file(file_name);
	}
	void init_from_file(const char *file_name);     // actually the same
	void sort_by(const int column_id);              // not implemented yet
	inline void sort_by(const string column_name)   // not implemented
	{
		sort_by(map_name_id[column_name]);
	}
	inline int number_of_columns()                  // returns number of columns
	{
		assert(columns_data.size() == columns_information.size());
		return (int)columns_data.size();
	}
	inline COLUMN_INFORMATION get_column_information(const int column_id)   // returns information about the column
	{
		return columns_information[column_id];
	}
	inline COLUMN_INFORMATION get_column_information(const string column_name)  // doesn't work
	{
		get_column_information(map_name_id[column_name]);
	}
	inline COLUMN& get_column(const int column_id)    // used to retrieve column data
	{
		return columns_data[column_id];
	}
	inline COLUMN& operator[](const int column_id)    // alias to get_column, works like    <object-name>[<column_id>]
	{
		return get_column(column_id);
	}
};

inline bool dbl_eq(double p1, double p2)
{
	return (fabs(p1-p2)<1e-12);
}

void FileParser::init_from_file(const char *file_name)
{
	FILE *file = fopen(file_name, "r");
	char str[10000];
	while (!feof(file))
	{
		fgets(str, 10000, file);
		char *cur = str;
		vector<double> values;
		int len_numbers = 0;
		while (*cur != 0)
		{
			char *next;
			double p = strtod(cur, &next);
			if (next == cur)
			{
				cur++;
			}
			else if (errno == ERANGE)
			{
				cur = next;
			}
			else
			{
				values.push_back(p);
				len_numbers += (int)(next-cur);
				cur = next;
			}
		}
		if (cur - str > 2*len_numbers)
		{
			continue;
		} else
		{
			if ((int)values.size() < number_of_columns())
				continue;
			if ((int)values.size() > number_of_columns())
			{
				columns_data.clear();
				columns_information.clear();
				columns_data.resize(values.size());
				columns_information.resize(values.size());
			}
			for (int i = 0; i < (int)values.size(); i++)
			{
				columns_data[i].push_back(values[i]);
			}
		}
	}
	fclose(file);
	for (int i = 0; i < number_of_columns(); i++)
	{
		columns_information[i] = make_column_info(columns_data[i]);
		columns_information[i].column_id = i;
	}
	return;
}

COLUMN_INFORMATION FileParser::make_column_info(const std::vector<double> &v)
{
	COLUMN_INFORMATION res;
	res.has_name = false;
	res.column_id = -1;
	res.name = "Unknown";
	res.number_of_elements = (int)v.size();
	res.regular = true;
	res.step = 0.0;
	res.max_value = DBL_MIN;
	res.min_value = DBL_MAX;
	if (v.size() == 0) return res;
	double prev = v[0];
	for (int i = 0; i < res.number_of_elements; i++)
	{
		res.max_value = max(res.max_value, v[i]);
		res.min_value = min(res.min_value, v[i]);
		if (dbl_eq(prev, v[i])) continue;
		if (dbl_eq(res.step, 0.0))
		{
			res.step = v[i] - prev;
		} else
		{
			if (!dbl_eq(v[i], prev+res.step))
				res.regular = false;
		}
		prev = v[i];
	}
	return res;
}

int main(int argc, char *argv[])
{
	FileParser("testfile.txt");
	return 0;
}
