SplitOperation = {
    command_line = "SPLIT",
    icon = "snap_intersection.svg"
}
SplitOperation.__index = SplitOperation

setmetatable(SplitOperation, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function SplitOperation:_init(id)
    Operations._init(self)

    self.selection = mainWindow:cadMdiChild():selection()

    message(tostring(#self.selection) .. " items selected")

    if(#self.selection == 1) then
	self.entity = self.selection[1]
        luaInterface:registerEvent('point', self)
        message("Give split point")
    else
        self.finished = true
        message('select only one item')
        luaInterface:triggerEvent('operationFinished')
    end
end

function SplitOperation:onEvent(eventName, event)
    if(eventName == "point" or eventName == "number") then
        self:newData(event["position"])
    end
end

function SplitOperation:newData(point)
	local entities = lc.entity.Splitable.splitHelper(self.entity, point)
	    if(#entities == 0) then
		self:close()
		return
	    end
	message('sucess')
	local b = lc.operation.EntityBuilder(mainWindow:cadMdiChild():document())

	b:appendEntity(self.entity)
	b:appendOperation(lc.operation.Push())
	b:appendOperation(lc.operation.Remove())

	b:processStack()
	for k, entity in pairs(entities) do
		b:appendEntity(entity)
	end
	b:execute()

	self:close()
end

function SplitOperation:close()
    if(not self.finished) then
        self.finished = true
        luaInterface:deleteEvent('point', self)
        luaInterface:triggerEvent('operationFinished')
    end
end
