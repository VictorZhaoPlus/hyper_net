local redis = require "serverd.redis"
local co = require "core.co"
local string = string
local assert = assert
local print = print
local pairs = pairs
local unpack = table.unpack
local select = select
local tostring = tostring
local type = type
local ipairs = ipairs
local tonumber = tonumber

_ENV = {}
-------------------------------------------- api --------------------------------------------------------
local readcmd = {}
readcmd[36] = function(data, left) -- '$'
	local bytes = tonumber(data)
	if bytes < 0 then
		return true,nil
	end
	return true, string.sub(left, 1, -3)
end

readcmd[43] = function(data, left) -- '+'
	return true, data
end

readcmd[45] = function(data, left) -- '-'
	return false, data
end

readcmd[58] = function(data, left) -- ':'
	-- todo: return string later
	return true, tonumber(data)
end

local read_response = function(msg)
	local pos = string.find(msg, "\r\n", 1, true)
	local firstchar = string.byte(msg)
	local data = string.sub(msg, 2, pos)
	local left = string.sub(msg, pos + 2)
	return readcmd[firstchar](data, left)
end

readcmd[42] = function(data, left)	-- '*'
	local n = tonumber(data)
	if n < 0 then
		return true, nil
	end
	local bulk = {}
	local noerr = true
	for i = 1,n do
		local ok, v = read_response(left)
		if ok then
			bulk[i] = v
		else
			noerr = false
		end
	end
	return noerr, bulk
end

local callback = function(succ, msg)
	assert(succ, "call failed")
	return read_response(msg)
end

local compose_command = function(...)
	local count = select("#", ...)
	local command = string.format("*%d\r\n", count)
	for i, v in ipairs{...} do
		local content = tostring(v)
		command = command..string.format("$%d\r\n", #content)
		command = command..string.format("%s\r\n", content)
	end
	return command
end

function get(thread, ...)
	local args = {...}
	return callback(co.wait(function(seq)
		redis.call(seq, thread, compose_command('GET', unpack(args)))
	end))
end

function set(thread, ...)
	local args = {...}
	return callback(co.wait(function(seq)
		redis.call(seq, thread, compose_command('SET', unpack(args)))
	end))
end

function delete(thread, ...)
	local args = {...}
	return callback(co.wait(function(seq)
		redis.call(seq, thread, compose_command('DEL', unpack(args)))
	end))
end

--------------------------------------------- callback -----------------------------------------------
redis.onRecv = function(seq, message)
	co.respone(seq, true, message)
end

redis.onFail = function(seq)
	co.respone(seq, false)
end

return _ENV