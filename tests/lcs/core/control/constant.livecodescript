script "CoreControlConstant"
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

constant kLiteral = left
constant kExprLiteral = (empty)
constant kSequenceLiteral = [ 1 ]
constant kArrayLiteral = { "value": 1 }
on TestConstant
	TestAssert "literal constant", kLiteral is "left"
	TestAssert "expr literal", kExprLiteral is empty
	TestAssert "seq literal", kSequenceLiteral is [ 1 ]
	TestAssert "arr literal", kArrayLiteral is { "value": 1 }
end TestConstant

on TestLocalConstant
	constant kLocalLiteral = left
	TestAssert "literal constant", kLocalLiteral is "left"

	constant kLocalExprLiteral = (empty)
	TestAssert "expr literal", kLocalExprLiteral is empty

	constant kLocalSequenceLiteral = [ 1 ]
	TestAssert "seq literal", kLocalSequenceLiteral is [ 1 ]
	TestAssert "seq literal (key)", kLocalSequenceLiteral[1] is 1

	constant kLocalArrayLiteral = { "value": 1 }
	TestAssert "arr literal", kLocalArrayLiteral is { "value": 1 }
	TestAssert "arr literal (key)", kLocalArrayLiteral["value"] is 1
end TestLocalConstant