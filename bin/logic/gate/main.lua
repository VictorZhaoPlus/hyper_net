local harbor = require "core.harbor"
local co = require "core.co"
local http = require "core.http"
local timer = require "core.timer"
local agent = require "core.agent"
local redis = require "core.redis"
local buff = require "serverd.buffer"

agent.set_parser(function(context, size)
	local used, msg = buff.readline(context, 0, size)
	if used == nil then
		return 0
	end
	local pos = string.find(msg, " ")
	if pos == nil then
		return -1
	end
	return used + 1, string.sub(msg, 1, pos - 1), string.sub(msg, pos + 1)
end)

agent.reg_open_listener(function(id)
	agent.send(id, "system connected\n")
end)

agent.register("GET", function(id, msg)
	print(msg)
	agent.send(id, "echo "..msg.."\n")
end)

agent.register("SET", function(id, msg)
	print(msg)
	agent.send(id, "fuck "..msg.."\n")
end)

harbor.register("add", function(a, b)
	print("add", a, b)
	harbor.ret(a + b)
end)

harbor.add_node_open_listener(function(nodeType, nodeId)
	if nodeType == 3 then
		local c = harbor.call(nodeType, nodeId, "add", true, 1, 2)
		print(c)
	else
		local content = http.get(0, "http://www.baidu.com")
		print(content)
		local a = redis.set(0, "test", "hello world")
		print(a)
		local b, r = redis.get(0, "test")
		print(r)
	end
end)

os.log("test log")