#ifndef _UTILS_H_
#define _UTILS_H_

#include <bitset>
#include <vector>
#include <cassert>

namespace psdkro
{

    constexpr int bitwidth = 64;

	enum class VarValue : std::uint8_t 
    {
		POSITIVE,   // var = 1
		NEGATIVE,   // var = 0
		DONTCARE    // var don't care
	}; 

	enum class ExpType : std::uint8_t 
    {
		pD,         // Positive Davio
        nD,         // Negative Davio
		Sh,         // Shannon
        
        // for incompletely specified function
        C0,         // case C0 = 0
        C1,         // case C1 = 0
        F0          // case F' = 0
	};

	struct cube
    {
		std::bitset<bitwidth> _polarity;
		std::bitset<bitwidth> _iscare;

		cube()
        {
			_polarity.reset();
			_iscare.reset();
		}

        // Return this cube in string type. '0'/'1'/'-'
		std::string str(const int nVar) const
		{
			std::string s;
			for (auto i = 0; i < nVar; ++i)
            {
				if (!_iscare.test(i))
					s.push_back('-');
				else if (_polarity.test(i))
					s.push_back('1');
				else
					s.push_back('0');
			}
			return s;
		}
	};
}

#endif
