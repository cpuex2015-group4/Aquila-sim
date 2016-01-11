function print_table(table)
	for i,v in ipairs(table) do
		print(tostring(i) .. ' : ' .. v)
	end
end

function split(str, delimiter)
	elms = {}
	elms_idx = 1
	elms[elms_idx] = ''
	for idx = 0, #str  do
		if str:sub(idx, idx) == delimiter then
			elms_idx = elms_idx + 1
			elms[elms_idx] = ''
		else
			elms[elms_idx] = elms[elms_idx] .. (str:sub(idx,idx))
		end
	end
	return elms
end

function parse_input(str)
	--[[
	args :
		string 
	returns :
		(int a, int b)
		a
		-------------------
		0 -> error
		1 -> next
		2 -> continue
		3 -> break

		b
		------------------
		when break
			break number
			
	--]]
	elms = split(str, ' ')
	ope = elms[1]
	arg = elms[2]
	if ope == 'next' or ope == 'n' then
		return 1, 0
	elseif ope == 'continue' or ope == 'c' then
		return 2, 0
	elseif ope == 'break' or ope == 'b' then
		return 3, tonumber(arg)
	end
end

function interpret()
	input = io.read()
	return parse_input(input)
end

