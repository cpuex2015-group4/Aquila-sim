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
	elms = split(str, ' ')
	ope = elms[1]
	arg = elms[2]
	if ope == 'next' or ope == 'n' then
		print('next')
	elseif ope == 'continue' or ope == 'c' then
		print('continue')
	elseif ope == 'break' or ope == 'b' then
		print('break')
	end
end

while true do
	input = io.read()
	res = parse_input(input)
end

