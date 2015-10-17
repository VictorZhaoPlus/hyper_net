local http = require "serverd.http"
local co = require "core.co"
local table = table
local string = string
local assert = assert
local print = print
local pairs = pairs
local unpack = table.unpack
local type = type

_ENV = {}
-------------------------------------------- api --------------------------------------------------------
local callback = function(succ, ...)
	assert(succ, "call failed")
	return ...
end

function get(thread, url, param)
	local uri = url
	if param ~= nil then
		local first = true
		for k, v in pairs(param) do
			if first then
				first = false
				uri = uri.."?"..k.."="..v
			else
				uri = uri.."&"..k.."="..v
			end
		end
	end
	
	return callback(co.wait(function(seq)
		http.get(thread, seq, uri)
	end))
end

function post(thread, url, param)
	local paramData = ''
	if param ~= nil then
		local first = true
		for k, v in pairs(param) do
			if first then
				first = false
				paramData = paramData..k.."="..v
			else
				paramData = paramData.."&"..k.."="..v
			end
		end
	end
	
	return callback(co.wait(function(seq)
		http.post(thread, seq, url, paramData)
	end))
end

--------------------------------------------- callback -----------------------------------------------
http.onSuccess = function(seq, message)
	co.respone(seq, true, message)
end

http.OnFailed = function(seq, errCode)
	co.respone(seq, false, errCode)
end

return _ENV