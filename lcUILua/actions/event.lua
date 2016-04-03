local event = {}

function event.register(eventName, callback, ...)
	event[eventName] = {}
	event[eventName].callback = callback
	if(... ~= nil) then
		event[eventName].args = {...}
	end
end

function event.trigger(eventName, ...)
	if(event[eventName]) then
		if(event[eventName].args) then
			event[eventName].callback(event[eventName].args, {...})
		else
			event[eventName].callback({...})
		end
	end
end

function event.delete(eventName)
	event[eventName] = nil
end

return event