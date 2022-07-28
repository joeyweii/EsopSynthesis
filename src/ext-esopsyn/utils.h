#include <bitset>
#include <vector>
#include <cassert>

namespace psdkro{

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

        // return cube in string form
		std::string str(const std::uint32_t nVar) const
		{
			std::string s;
			for (auto i = 0; i < nVar; ++i) {
				if (!mask.test(i))
					s.push_back('-');
				else if (polarity.test(i))
					s.push_back('1');
				else
					s.push_back('0');
			}
			return s;
		}

        // return the ith literal in char form
        char lit(const std::uint32_t i) const
        {
            if(!mask.test(i)) 
                return '-';
            else if(polarity.test(i))
                return '1';
            else    
                return '0';
        }
	};
}
