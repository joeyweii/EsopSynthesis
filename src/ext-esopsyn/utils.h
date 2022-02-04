#include <bitset>
#include <vector>

namespace bddpsdkro{

	// Modify to fit the number of variables
	constexpr int bitwidth = 64;

	enum var_value : std::uint8_t {
		POSITIVE, // xi = 1
		NEGATIVE, // xi = 0
		UNUSED // xi don't care
	};

	enum exp_type : std::uint8_t {
		POSITIVE_DAVIO,
		NEGATIVE_DAVIO,
		SHANNON
	};

	struct cube{
		std::bitset<bitwidth> polarity;
		std::bitset<bitwidth> mask;

		cube(){
			polarity.reset();
			mask.reset();
		}

		std::string str(const std::uint32_t n_inputs) const
		{
			std::string s;
			for (auto i = 0; i < n_inputs; ++i) {
				if (!mask.test(i)) {
					s.push_back('-');
				} else if (polarity.test(i))
					s.push_back('1');
				else
					s.push_back('0');
			}
			return s;
		}
	};
}