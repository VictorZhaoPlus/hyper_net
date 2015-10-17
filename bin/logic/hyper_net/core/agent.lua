local agent = require "serverd.agent"
local string = string
local assert = assert
local print = print
local pairs = pairs
local unpack = table.unpack

_ENV = {}
local open_listener = {}
local close_listener = {}

-------------------------------------------- api --------------------------------------------------------
function reg_open_listener(func)
	open_listener[#open_listener + 1] = func
end

function reg_close_listener(func)
	close_listener[#close_listener + 1] = func
end

parser = nil
function set_parser(func)
	parser = func
end

local proto = {}
function register(msgId, func)
	if proto[msgId] == nil then
		proto[msgId] = {}
	end
	proto[msgId][#proto[msgId] + 1] = func
end

function send(id, msgId, context)
	agent.send(id, msgId, context)
end

function kick(id)
	agent.kick(id)
end

--------------------------------------------- callback -----------------------------------------------
agent.onOpen = function(id)
	for i = 1, #open_listener do
		open_listener[i](id)
	end
end

agent.onRecv = function(id, context, size)
	assert(parser, "where is agent packet parser")
	local used, msgId, msg = parser(context, size)
	if used > 0 then
		local funces = proto[msgId]
		if funces ~= nil then
			for i = 1, #funces do
				funces[i](id, msg)
			end
		end
	end

	return used
end

agent.onClose = function(id)
	for i = 1, #close_listener do
		close_listener[i](id)
	end
end

return _ENV