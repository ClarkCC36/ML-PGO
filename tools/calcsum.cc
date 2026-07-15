// ML-PGO: GCOV Profile Data Integrity Tool

#include <stdio.h>
#include <string>
#include <vector>
#include <assert.h>

using namespace std;

static unsigned histogram_index (long long value)
{
	unsigned long long v = (unsigned long long)value;
	unsigned r = 0;
	unsigned prev2bits = 0;
	if (v > 0)
		r = 63 - __builtin_clzll (v);
	if (r < 2)
		return (unsigned)value;
	assert (r < 64);
	prev2bits = (v >> (r - 2)) & 0x3;
	return (r - 1) * 4 + prev2bits;
}

static int read_file(const char *filename, vector<char> &out)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp)
	{
		fprintf(stderr, "[!] Fail to read: %s\n", filename);
		return 1;
	}
	char buf[4096];
	size_t sz;
	while ((sz = fread(buf, 1, 4096, fp)))
		out.insert(out.end(), buf, buf + sz);
	fclose(fp);
	return 0;
}

static void split_lines(const vector<char> &in, vector<string> &out)
{
	size_t pos = 0;
	for (size_t i = 0; i < in.size(); ++i)
	{
		if (in[i] != '\r' && in[i] != '\n')
			continue;
		out.push_back(string(in.begin() + pos, in.begin() + i));
		if (in[i] == '\r' && i + 1 < in.size() && in[i + 1] == '\n')
			i++;
		pos = i + 1;
	}
	if (pos < in.size())
		out.push_back(string(in.begin() + pos, in.end()));
}

static unsigned num;
static long long sum_all, max_val;
struct bucket
{
	unsigned num;
	long long min, sum;
} histogram[256];

static void histogram_insert(long long value)
{
	unsigned i = histogram_index(value);
	histogram[i].num++;
	histogram[i].sum += value;
	if (value < histogram[i].min)
		histogram[i].min = value;
}

#define GCOV_DATA_MAGIC		  0x67636461u
#define GCOV_TAG_FUNCTION		0x01000000u
#define GCOV_TAG_COUNTER_BASE	0x01a10000u
#define GCOV_TAG_PROGRAM_SUMMARY 0xa3000000u

int process(const unsigned *v, int n)
{
	int state = 1;
	unsigned count;
	for (int i = 0; i < n; ++i)
	{
		unsigned val = v[i];
		switch (state)
		{
			case 1:
				if (val != GCOV_DATA_MAGIC)
				{
					fprintf(stderr, "[!] GCOV_DATA_MAGIC mismatch: 0x%x\n", val);
					return 1;
				}
				i += 2;
				state = 2;
				break;
			case 2:
				if (i == n - 1 && val)
				{
					fprintf(stderr, "[!] Single last tag: 0x%x\n", val);
					return 2;
				}
				if (val == GCOV_TAG_FUNCTION)
					i += 1 + v[i + 1];
				else if (val == GCOV_TAG_COUNTER_BASE)
				{
					if (v[i + 1] % 2)
					{
						fprintf(stderr, "[!] Invalid length: %d\n", v[i + 1]);
						return 3;
					}
					count = v[i + 1] / 2;
					num += count;
					if (count)
						state = 3;
					i++;
				}
				else if (val)
				{
					if (!(val & 0xffff) && ((val - GCOV_TAG_COUNTER_BASE) >> 17) < 9)
						i += 1 + v[i + 1];
					else
					{
						fprintf(stderr, "[!] Unknown tag: 0x%x\n", val);
						return 4;
					}
				}
				break;
			case 3:
				long long ctr = v[++i];
				ctr = (ctr << 32) + val;
				histogram_insert(ctr);
				sum_all += ctr;
				if (ctr > max_val)
					max_val = ctr;
				if (--count == 0)
					state = 2;
				break;
		}
		if (i >= n)
		{
			fprintf(stderr, "[!] Overflow\n");
			return 5;
		}
	}
	if (state == 3)
	{
		fprintf(stderr, "[!] Overflow\n");
		return 6;
	}
	return 0;
}

unsigned mask[8], hcount = 0;

int write_file(const vector<char> in, const char *filename)
{
	if (in.size() < 12)
	{
		fprintf(stderr, "[!] Not enough to write\n");
		return 1;
	}
	FILE *fp = fopen(filename, "wb");
	if (!fp)
	{
		fprintf(stderr, "[!] Fail to write: %s\n", filename);
		return 2;
	}
	fwrite(in.data(), 1, 12, fp);
	unsigned tmp[11] = {
		GCOV_TAG_PROGRAM_SUMMARY,
		17 + hcount * 5,
		0,
		num,
		1,
		(unsigned)sum_all,
		(unsigned)(sum_all >> 32),
		(unsigned)max_val,
		(unsigned)(max_val >> 32),
		(unsigned)max_val,
		(unsigned)(max_val >> 32)
	};
	fwrite(tmp, 1, 44, fp);
	fwrite(mask, 1, 32, fp);
	for (int i = 0; i < 256; ++i)
	{
		if (histogram[i].num <= 0)
			continue;
		unsigned tmp2[5] = {
			histogram[i].num,
			(unsigned)(histogram[i].min),
			(unsigned)(histogram[i].min >> 32),
			(unsigned)(histogram[i].sum),
			(unsigned)(histogram[i].sum >> 32)
		};
		fwrite(tmp2, 1, 20, fp);
	}
	fwrite(in.data() + 12, 1, in.size() - 12, fp);
	fclose(fp);
	return 0;
}

int main(int argc, char **argv)
{
	vector<char> content;
	if (argc != 2 || read_file(argv[1], content))
	{
		fprintf(stderr, "USAGE:\n  %s <Input File List>\n", argv[0]);
		return 1;
	}
	for (int i = 0; i < 256; ++i)
		histogram[i].min = 0x7fffffffffffffffll;
	vector<string> lines;
	split_lines(content, lines);
	vector<vector<char> > v(lines.size());
	for (size_t i = 0; i < lines.size(); ++i)
	{
		const char *filename = lines[i].c_str();
		if (read_file(filename, v[i]))
			return 2;
		if (v[i].size() % 4)
		{
			fprintf(stderr, "[!] File size mismatch: %s\n", filename);
			return 3;
		}
		fprintf(stderr, "[.] Processing %s\n", filename);
		if (process((const unsigned *)v[i].data(), v[i].size() / 4))
			return 4;
	}
	fprintf(stderr, "[.] num: %d\n", num);
	fprintf(stderr, "[.] sum_all: %lld\n", sum_all);
	fprintf(stderr, "[.] max_val: %lld\n", max_val);
	for (int i = 0; i < 256; ++i)
	{
		if (histogram[i].num <= 0)
			continue;
		hcount++;
		mask[i / 32] |= 1 << (i % 32);
		fprintf(stderr, "[.] histogram(%03d): num=%d, min=%lld, sum=%lld\n", i, histogram[i].num, histogram[i].min, histogram[i].sum);
	}
	for (size_t i = 0; i < lines.size(); ++i)
	{
		if (write_file(v[i], lines[i].c_str()))
			return 5;
	}
	fprintf(stderr, "[.] Files processed: %d\n", (int)lines.size());
	return 0;
}

