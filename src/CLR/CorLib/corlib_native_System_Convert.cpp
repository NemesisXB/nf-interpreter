//
// Copyright (c) 2017 The nanoFramework project contributors
// Portions Copyright (c) Microsoft Corporation.  All rights reserved.
// See LICENSE file in the project root for full license information.
//

#include "corlib_native.h"
#include <ctype.h>

HRESULT Library_corlib_native_System_Convert::NativeToInt64___STATIC__I8__STRING__BOOLEAN__I8__I8__I4( CLR_RT_StackFrame& stack )
{
    NANOCLR_HEADER();
    {
        int64_t result = 0;

		char* str = (char*)stack.Arg0().RecoverString();
        signed int radix = stack.Arg4().NumericByRef().s4;

      #if (SUPPORT_ANY_BASE_CONVERSION == TRUE)
        // suport for conversion from any base

        int64_t zero = 0;

        bool isSigned = (bool)stack.Arg1().NumericByRef().u1;
        long long minValue = stack.Arg2().NumericByRef().s8;
        long long maxValue = stack.Arg3().NumericByRef().s8;

        result = isSigned ? strtoll(str, nullptr, radix) : (long long) strtoull(str, nullptr, radix);

        stack.SetResult_I8 ((result > maxValue || result < minValue) ? zero : result);
        
      #else
        // support for conversion from base 10 and 16 (partial)

        if(radix == 10)
        {
            // conversion from base 10

            // check for minus sign
            if(*str == '-')
            {
                // call GetIntegerPart() in 'guess' mode by throwing a crazy number for the length
                result = GetIntegerPart((str + 1), 99);

                // is negative
                result *= -1;
            }
            // check for plus sign
            else if(*str == '+')
            {
                // call GetIntegerPart() in 'guess' mode by throwing a crazy number for the length
                result = GetIntegerPart((str + 1), 99);
            }
            else
            {
                // call GetIntegerPart() in 'guess' mode by throwing a crazy number for the length
                result = GetIntegerPart(str, 99);
            }

        }
        else if(radix == 16)
        {
            // conversion from base 16
            result = GetIntegerFromHexString(str);
        }
        else
        {
            // all other bases are not supported
            return stack.NotImplementedStub();
        }

        stack.SetResult_I8(result);

      #endif // defined(SUPPORT_ANY_BASE_CONVERSION)
	}
    NANOCLR_NOCLEANUP_NOLABEL();
}

HRESULT Library_corlib_native_System_Convert::NativeToDouble___STATIC__R8__STRING( CLR_RT_StackFrame& stack )
{
    NANOCLR_HEADER();
    {
        char* str = (char*)stack.Arg0().RecoverString();

  #if (SUPPORT_ANY_BASE_CONVERSION == TRUE)
        // suport for conversion from any base

        stack.SetResult_R8 (strtod(str, nullptr));
    }
    NANOCLR_NOCLEANUP_NOLABEL();

  #else
        // support for conversion from base 10 and 16 (partial)
        // in this particular function the base isn't relevant

        int length = 0;
        bool hasMinusSign = false;
        bool hasPlusSign = false;
        int decimalPoint = -1;
        int exponentialSign = -1;
        bool hasMinusExponentialSign = false;
        bool hasPlusExponentialSign = false;
        double returnValue = 0.0;

        // first pass, get count of decimal places, integer part and check for valid chars
        char* temp  = str;
        while (*temp != '\0')
        {
            switch (*temp)
            {
                case '-':
                    if (exponentialSign == -1)
                    {
                        if (length == 0)
                        {
                            hasMinusSign = true;
                        }
                        else 
                        {
                            // found a minus signal NOT at the start of the string
                            NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_PARAMETER);
                        }
                    }
                    else
                    {
                        if (length == exponentialSign + 1)
                        {
                            hasMinusExponentialSign = true;
                        }
                        else 
                        {
                            // found a minus signal NOT at the start of the exponent
                            NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_PARAMETER);
                        }
                    }
                    break;

                case '+':
                    if (exponentialSign == -1)
                    {
                        if (length == 0)
                        {
                            hasPlusSign = true;
                        }
                        else 
                        {
                            // found a plus signal NOT at the start of the string
                            NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_PARAMETER);
                        }
                    }
                    else
                    {
                        if (length == exponentialSign + 1)
                        {
                            hasPlusExponentialSign = true;
                        }
                        else 
                        {
                            // found a plus signal NOT at the start of the exponent
                            NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_PARAMETER);
                        }
                    }
                    break;

                case '.':
                    if (decimalPoint == -1)
                    {
                        decimalPoint = length;
                    }
                    else
                    {
                        // already found a decimal point, can't have another
                        NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_PARAMETER);
                    }
                    break;

                case 'e':
                case 'E':
                    if (exponentialSign == -1)
                    {
                        exponentialSign = length;
                    }
                    else
                    {
                        // already found a exponential sign, can't have another
                        NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_PARAMETER);
                    }
                    break;

                default:
                    if (*temp < '0' && *temp > '9')
                    {
                        // there is an invalid char in the string
                        NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_PARAMETER);
                    }
            }
            length++;
            temp++;
        }

        //  now parse the string according to it's format
        int endOrExponentialPart = exponentialSign == -1 ? length : exponentialSign;
        if (decimalPoint == -1)
        {
            // string doesn't have fractional part, treat as integer
            returnValue = GetIntegerPart(str, endOrExponentialPart);
        }
        else if (decimalPoint == 0)
        {
            // string starts with the decimal point, only has fractional part
            returnValue = GetDoubleFractionalPart((str+decimalPoint+1), (endOrExponentialPart-decimalPoint-1));
        }
        else if (hasMinusSign || hasPlusSign)
        {
            // string starts with sign and...
            
            if(decimalPoint == 1)
            {
                // ... is followed by a decimal point, only has fractional part
                returnValue = GetDoubleFractionalPart((str+decimalPoint+1), (endOrExponentialPart-decimalPoint-1));
            }
            else
            {
                // ... has integer and fractional parts
                returnValue = GetIntegerPart(str+1, decimalPoint-1);
                returnValue = (returnValue + GetDoubleFractionalPart((str+decimalPoint+1), (endOrExponentialPart-decimalPoint-1)));
            }

            if (hasMinusSign)
            {
                returnValue *= -1;
            }
        }
        else
        {
            // string has integer and fractional parts
            returnValue = GetIntegerPart(str, decimalPoint);
            returnValue = returnValue + GetDoubleFractionalPart((str+decimalPoint+1), (endOrExponentialPart-decimalPoint-1));
        }

        // exponential part found?
        if (exponentialSign != -1)
        {
            // advance by one if a sign (+ or -) is after the exponential sign
            if (hasMinusExponentialSign || hasPlusExponentialSign) exponentialSign++;
            // get the exponential part
            int exponent = GetIntegerPart((str+exponentialSign+1), (length-exponentialSign-1));
            // each time multiply or divide by 10
            for (int i = 0; i < exponent; i++)
            {
                if (hasMinusExponentialSign)
                {
                    returnValue /= 10;
                }
                else
                {
                    returnValue *= 10;
                }
            }
        }

        stack.SetResult_R8(returnValue);
	}
    NANOCLR_NOCLEANUP();

  #endif // defined(SUPPORT_ANY_BASE_CONVERSION)        

}

HRESULT Library_corlib_native_System_Convert::ToBase64String___STATIC__STRING__SZARRAY_U1__I4__I4__BOOLEAN( CLR_RT_StackFrame& stack )
{
    NANOCLR_HEADER();

    NANOCLR_SET_AND_LEAVE(stack.NotImplementedStub());

    NANOCLR_NOCLEANUP();
}

HRESULT Library_corlib_native_System_Convert::FromBase64CharArray___STATIC__SZARRAY_U1__SZARRAY_CHAR__I4( CLR_RT_StackFrame& stack )
{
    NANOCLR_HEADER();

    NANOCLR_SET_AND_LEAVE(stack.NotImplementedStub());

    NANOCLR_NOCLEANUP();
}

double Library_corlib_native_System_Convert::GetDoubleFractionalPart(char* str, int length)
{
    double place = 1;
    double returnValue = 0.0;

    for (int i = 0; i < length; i++)
    {
        // move decimal place to the right
        place /= 10.0;

        returnValue += ((int)(*str++) - '0') * place;
    }

    return returnValue;
}

int64_t Library_corlib_native_System_Convert::GetIntegerPart(char* str, int length)
{
    int64_t returnValue = 0;

    for (int i = 0; i < length; i++)
    {
        returnValue = returnValue * 10 + (*str - '0');
        str++;

        // check for terminator, in case this is being called in 'guess' mode
        if(*str == '\0')
        {
            break;
        }
    }

    return returnValue;
}

int64_t Library_corlib_native_System_Convert::GetIntegerFromHexString(char* str)
{
    int64_t returnValue = 0;

	if ((*str == '0') && (*(str+1) == 'x'))
    {
        // there a 0x at the begining of the string, so move pointer forward 2 notches
        str += 2;
    }

	while (*str != '\0')
	{
		char c = toupper(*str++);

		if ((c < '0') || (c > 'F') || ((c > '9') && (c < 'A')))
        {
            // there is an invalid char in the string
            break;
        }

		c -= '0';

		if (c > 9)
        {
			c -= 7;
        }

		returnValue = (returnValue << 4) + c;
	}

	return returnValue;
}
