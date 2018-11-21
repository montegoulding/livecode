script "CoreControlType"
/*
Copyright (C) 2018 LiveCode Ltd.

This file is part of LiveCode.

LiveCode is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License v3 as published by the Free
Software Foundation.

LiveCode is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with LiveCode.  If not see <http://www.gnu.org/licenses/>.  */

constant kConstantOne = (1 _as_number_)
constant kConstantOneString = "1"

operator _as_type_ test.missing_func pValue
	return missingFunction()
end _as_type_ test.missing_func

operator _as_type_ test.strnum pValue
	if pValue is strictly a real then
		return pValue & ""
	end if
	if pValue is a number then
		return pValue
	end if
	return _type_error_("not a numeric string")
end _as_type_ test.strnum

operator _as_type_ test.number pValue
	if pValue is strictly a real or \
		pValue is strictly an integer then
		return pValue _with_type_ test.number
	end if
	if pValue is a number then
		return (pValue + 0) _with_type_ test.number
	end if
	if pValue is "?" then
		return ("inf" + 0) _with_type_ test.number
	end if
	return _type_error_("not a number")
end _as_type_ test.number

operator boolean test.number pValue
	return pValue is not 0
end boolean test.number

operator string test.number pValue
	if pValue is ("inf" _as_number_) then
		return "?"
	end if
	get format("%lf", pValue)
	repeat while the last codeunit of it is "0"
		delete the last codeunit of it
	end repeat
	if the last codeunit of it is "." then
		delete the last codeunit of it
	end if
	return it
end string test.number

operator data test.number pValue
	return binaryEncode("d", pValue)
end data test.number

operator array test.number pValue
	return [pValue]
end array test.number

command startup
	wait 1 second
	TestType
	quit 0
end startup

on TestType
	get empty _as_type_ test.missing_func
	write (("inf" + 0) _as_type_ test.number) & "" & return to stdout
	get empty _with_type_ test.type
	get empty _is_type_ test.type
	get (empty _with_type_ test.type) _as_type_ test.type
	get empty _has_type_ test.type
	get _strict_type_of_(empty)
	get _type_of_(empty)
	get try(_type_error_("oh no!"), "")
end TestType

on TestStrictTypeOf
	local tUninitializedVar
	TestAssert "strict type of null is nothing", _strict_type_of_(tUninitializedVar) is "nothing"
	TestAssert "strict type of true is boolean", _strict_type_of_(not false) is "boolean"
	TestAssert "strict type of false is boolean", _strict_type_of_(not true) is "boolean"
	TestAssert "strict type of nativeCharToNum(a) is number", _strict_type_of_(nativeCharToNum("a")) is "number"
	TestAssert "strict type of pi is number", _strict_type_of_(pi) is "number"
	TestAssert "strict type of empty is string", _strict_type_of_(empty) is "string"
	TestAssert "strict type of numToByte(0) is data", _strict_type_of_(numToByte(0)) is "data"
	TestAssert "strict type of array literal is array", _strict_type_of_({"a": 0}) is "array"
	TestAssert "strict type of sequence literal is array", _strict_type_of_(["a"]) is "array"
end TestStrictTypeOf

on TestAsStrictType
	TestAssert "as boolean is boolean", (true _as_boolean_) is strictly a boolean
	TestAssert "as number is number", (0 _as_number_) is strictly a number
	TestAssert "as string is string", ((1+0) _as_string_) is strictly a string
	TestAssert "as data is data", ((1) _as_data_) is strictly a binary string
	TestAssert "as array is array", ((1 _as_type_ test.number) _as_array_) is strictly an array
end TestAsStrictType

on TestConcreteType
	local tNumber, tOtherNumber
	put (0 + 0) _with_type_ foo into tNumber
	TestAssert "number with type foo has type foo", tNumber _has_type_ foo
	TestAssert "number with type has typeof foo", _type_of_(tNumber) is "foo"
	put tNumber into tOtherNumber
	TestAssert "other of number with type foo has type foo", tOtherNumber _has_type_ foo
	add 1 to tNumber
	TestAssert "mutation of number with type has no type", _type_of_(tNumber) is empty

	local tString, tOtherString
	put "foobar" _with_type_ foo into tString
	TestAssert "string with type foo has type foo", tString _has_type_ foo
	TestAssert "string with type has typeof foo", _type_of_(tString) is "foo"
	put tString into tOtherString
	TestAssert "other of string with type foo has type foo", tOtherString _has_type_ foo
	put empty after tString
	TestAssert "mutation of string with other with type has no type", _type_of_(tString) is empty
	put ("s" & random(100)) _with_type_ foo into tString
	put empty after tString
	TestAssert "mutation of string without other with type has no type", _type_of_(tString) is empty

	local tArray, tOtherArray
	put [1] _with_type_ foo into tArray
	TestAssert "array with type foo has type foo", tArray _has_type_ foo
	TestAssert "array with type has typeof foo", _type_of_(tArray) is "foo"
	put tArray into tOtherArray
	TestAssert "other of string with type foo has type foo", tOtherArray _has_type_ foo
	put 2 into tArray[1]
	TestAssert "mutation of array with other with type has no type", _type_of_(tArray) is empty
	put [random(100)] _with_type_ foo into tArray
	add 1 to tArray[1]
	TestAssert "mutation of array without other with type has no type", _type_of_(tArray) is empty
end TestConcreteType

on TestAsType
	TestAssert "as type strnum of number", try((100 + 0) _as_type_ test.strnum, empty) is "100"
	TestAssert "as type strnum of array", try([1 + 0] _as_type_ test.strnum, "failed") is "failed"

	TestAssert "as type number of real", try((100 / 200) _as_type_ test.number, empty) is 100 / 200
	TestAssert "as type number of string", try("100" _as_type_ test.number, "failed") is 100 + 0
	TestAssert "as type number of ?", try("?" _as_type_ test.number, "failed") is ("inf" + 0)
	TestAssert "string number of +inf", try((("inf" + 0) _as_type_ test.number) & "", "failed") is "?"
end TestAsType

on TestBoolean
	TestAssert "boolean of number 0 is false", not ((0 _as_type_ test.number) _as_boolean_)
	TestAssert "boolean of number 1 is true", ((1 _as_type_ test.number) _as_boolean_)
end TestBoolean

on TestString
	TestAssert "string of number 0 is '0'", ((0 _as_type_ test.number) _as_string_) is strictly "0"
	TestAssert "string of number +inf is '?'", (("inf" _as_type_ test.number) _as_string_) is strictly "?"
end TestString

on TestData
	TestAssert "data of number 0 is binary encoded 0.0", ((0 _as_type_ test.number) _as_data_) is strictly binaryEncode("d", 0)
end TestData

on TestArray
	TestAssert "array of number 0 is [0]", ((0 _as_type_ test.number) _as_array_) is strictly [ 0 _as_number_ ]
end TestArray

on TestNonLaxHandlers
	TestAssert "non-lax handler too few args", not try(_NonLaxHandler(), false)
	TestAssert "non-lax handler too many args", not try(_NonLaxHandler(1,2), false)
	TestAssert "non-lax handler correct args", try(_NonLaxHandler(1), false)
	TestAssert "non-lax-handler variadic args", try(_NonLaxHandlerVariadic(1, 2, 3), 0) is 3
	TestAssert "non-lax-handler unnamed variadic args", try(_NonLaxHandlerUnnamedVariadic(1, 2, 3), 0) is 3
	TestAssert "non-lax-handler default not provided", try(_NonLaxHandlerDefault(1), empty) is "default"
	TestAssert "non-lax-handler default no expression", try(_NonLaxHandlerDefault(1, ), empty) is "default"
	TestAssert "non-lax-handler default provided", try(_NonLaxHandlerDefault(1, 2), empty) is 2
	TestAssert "non-lax-handler too few default", try(_NonLaxHandlerDefault(), empty) is empty
	TestAssert "non-lax-handler too many default", try(_NonLaxHandlerDefault(1, 2, 3), empty) is empty
	TestAssert "non-lax-handler too few default", try(_NonLaxHandlerMiddleDefault(1), empty) is empty
	TestAssert "non-lax-handler too many default", try(_NonLaxHandlerMiddleDefault(1, 2, 3, 4), empty) is empty
	TestAssert "non-lax-handler middle default", try(_NonLaxHandlerMiddleDefault(1, ,3), empty) is "default"
	TestAssert "non-lax-handler nothing doesn't trigger default", try(_NonLaxHandlerMiddleDefault(1, _nothing_ ,3), empty) is strictly _nothing_
	TestAssert "non-lax-handler with all trailing", _NonLaxHandlerForTrailing(... [1, 2, 3, 4, 5]) is [1, [2, 3, 4, 5]]
	TestAssert "non-lax-handler with most trailing", _NonLaxHandlerForTrailing(1, ... [2, 3, 4, 5]) is [1, [2, 3, 4, 5]]
	TestAssert "non-lax-handler with some trailing", _NonLaxHandlerForTrailing(1, 2, 3, ... [4, 5]) is [1, [2, 3, 4, 5]]
	TestAssert "non-lax-handler with all trailing unnamed", _NonLaxHandlerForTrailingUnnamed(... [1, 2, 3, 4, 5]) is [1, 2, 3, 4, 5]
	TestAssert "non-lax-handler with most trailing unnamed", _NonLaxHandlerForTrailingUnnamed(1, ... [2, 3, 4, 5]) is [1, 2, 3, 4, 5]
	TestAssert "non-lax-handler with all trailing unnamed", _NonLaxHandlerForTrailingUnnamed(1, 2, 3, ... [4, 5]) is [1, 2, 3, 4, 5]
end TestNonLaxHandlers

function _NonLaxHandler @(copy) pOne
	return true
end _NonLaxHandler

function _NonLaxHandlerVariadic pArgsV ...
	return the number of elements in pArgsV
end _NonLaxHandlerVariadic

function _NonLaxHandlerUnnamedVariadic ...
	return the paramCount
end _NonLaxHandlerUnnamedVariadic

function _NonLaxHandlerForTrailing pOne, pArgsV...
	return [ pOne, pArgsV ]
end _NonLaxHandlerForTrailing

function _NonLaxHandlerForTrailingUnnamed ...
	TestDiagnostic the params
	return params(1)
end _NonLaxHandlerForTrailingUnnamed

function _NonLaxHandlerDefault pOne, pTwo default "default"
	return pTwo
end _NonLaxHandlerDefault

function _NonLaxHandlerMiddleDefault pOne, pTwo default "default", pThree
	return pTwo
end _NonLaxHandlerMiddleDefault

on TestTypedHandlers
	TestAssert "typed handler correct type (number)", try(_TypedHandler(100 + 0), false)
	TestAssert "typed handler correct type (numeric string)", try(_TypedHandler("100.2"), false)
	TestAssert "typed handler incorrect type (array)", not try(_TypedHandler([1]), false)
	TestAssert "typed return handler correct type (number)", try(_TypedHandler(100 + 0), false)
	TestAssert "typed return handler incorrect type (array)", not try(_TypedHandler([1]), false)
end TestTypedHandlers

function _TypedHandler pOne as test.strnum
	return pOne is strictly a string
end _TypedHandler

function _TypedReturnHandler ( pValue ) as test.strnum
	return pValue
end _TypedReturnHandler

on TestScriptSignatures
	local tSignatures
	put the revScriptSignatures of me into tSignatures
	repeat for each element tElement in tSignatures
		TestDiagnostic _valueFormat(tElement)
	end repeat
end TestScriptSignatures

private function _valueFormat pValue
	if isSeq(pValue) then
		local tSeqValue
		put _seqFormat(pValue) into tSeqValue
		return tSeqValue
	else if pValue is an array then
		local tMapValue
		put _mapFormat(pValue) into tMapValue
		return tMapValue
	else if pValue is among the items of "true,false" then
		return pValue
	else if pValue is strictly a number then
		return pValue
	else if pValue is strictly a binary string then
		return format("<%s>", hexEncode(pValue))
	end if
	return format("\"%s\"", stringEscape(pValue))
end _valueFormat

private function _seqFormat pSeq
	local tElements
	repeat for each element tElement in pSeq
		/* ENGINE-BUG: If this is
		 *    seqPushOntoBack tElements, _valueFormat(tElement))
		 * THen strange things start to happen - i.e. an odd error. */
		local tValue
		put _valueFormat(tElement) into tValue
		seqPushOntoBack tElements, tValue
	end repeat
	combine tElements with ", "
	return format("[ %s ]", tElements)
end _seqFormat

private function _mapFormat pMap
	local tElements
	repeat for each key tKey in pMap
		/* ENGINE-BUG: If this is
		 *    seqPushOntoBack tElements, format("%s: %s", tKey, _valueFormat(pMap[tKey]))
		 * THen strange things start to happen - i.e. an odd error. */
		local tValue
		put _valueFormat(pMap[tKey]) into tValue
		seqPushOntoBack tElements, format("%s: %s", tKey, tValue)
	end repeat
	combine tElements with ", "
	return format("{ %s }", tElements)
end _mapFormat

private command seqPushOntoBack @xSeq, pElement
	put pElement into xSeq[the number of elements in xSeq + 1]
end seqPushOntoBack

private function isSeq pValue
	return _strict_type_of_(pValue) is "array" and item 2 of the extents of pValue is the number of elements in pValue
end isSeq

private function stringEscape pValue
	replace "\" with "\\" in pValue
	replace return with "\n" in pValue
	return pValue
end stringEscape
