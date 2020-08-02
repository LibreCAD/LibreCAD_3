TrimOperation = {
    command_line = "TRIM",
    icon = "modifytrim.png"
}
TrimOperation.__index = TrimOperation
-----------------------------------
-- Use as
-- First move entities to trim in self.toTrim
-- then split it using function TrimOperation:trimPoint(point)
-- finally commit it using function TrimOperation:replaceEntities()
-----------------------------------
setmetatable(TrimOperation, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

-- Replace old entities with new entities in document
function TrimOperation:replaceEntities()
	oldEntities=self.oldEntities
	newEntities=self.newEntities

	if (#oldEntities==0) then--no operation
		return
	end
	
	local b = lc.operation.EntityBuilder(mainWindow:cadMdiChild():document())
	for k, entity in pairs(oldEntities) do
		b:appendEntity(entity)
	end
	b:appendOperation(lc.operation.Push())
	b:appendOperation(lc.operation.Remove())

	b:processStack()
	for k, entity in pairs(newEntities) do
		b:appendEntity(entity)
	end
	b:execute()
	message('sucess')
end

function TrimOperation:_init(id)
    Operations._init(self)

    self.limit = nil
    self.intersectionPoints = {}
    self.newEntities = {} -- entities to add
    self.oldEntities = {} -- entities to remove

    self.selection = mainWindow:cadMdiChild():selection()
    --lets copy to totrim
    self.toTrim = self.selection
    
    message("<b>TRIM</b>")
    message(tostring(#self.toTrim) .. " items selected")

    if(#self.toTrim == 1) then
        luaInterface:registerEvent('point', self)
	luaInterface:registerEvent("selectionChanged", self)
        message("Select limit entity or Give split point")
    elseif (#self.toTrim > 1) then
    	self:selectionChanged() -- this should not be done this way
    	-- for now split in intersection
    else
        self.finished = true
        message('select some entities to trim')
        luaInterface:triggerEvent('operationFinished')
    end
end

function TrimOperation:onEvent(eventName, data)
    if(eventName == "selectionChanged") then
        self:selectionChanged()-- this is not working
    elseif(eventName == "point") then
        self:newPoint(data["position"])
    end
end

-- return newEntities based on trim point
function TrimOperation:trimPoint(point)
	--do not use table.remove
	--print(#self.toTrim)
	local nextToTrim = {}
	local nextNewEntities={}
	for k, entity in pairs(self.toTrim) do
		local entities = lc.entity.Splitable.splitHelper(entity, point)
		    if(#entities > 0) then
		    	--split sucessful
		    	print('split on main entity')
			for k2, entity2 in pairs(entities) do
				table.insert(nextNewEntities, entity2)
			end
			table.insert(self.oldEntities, entity) -- remove old
		    else
		    	table.insert(nextToTrim, entity)
	    	    end
	end	
	self.toTrim=nextToTrim
	
	-- if it splits new created entity
	for k, entity in pairs(self.newEntities) do
		local entities = lc.entity.Splitable.splitHelper(entity, point)
		    if(#entities > 0) then
		    	--split sucessful
		    	print('split on new entity')
			for k2, entity2 in pairs(entities) do
				table.insert(nextNewEntities, entity2)
			end
		    else
			table.insert(nextNewEntities, entity)
	    	    end
	end
	self.newEntities = nextNewEntities
end

-- Split point given
function TrimOperation:newPoint(point)
	--print(point)
	self:trimPoint(point)
	self:replaceEntities()
	self:close()
end

-- limit entity selected
function TrimOperation:selectionChanged()
    self.selection = mainWindow:cadMdiChild():selection()
    --print(#self.selection)
    local intersect = lc.maths.IntersectMany(self.selection)
    self.intersectionPoints = intersect:result()
	--print(#self.intersectionPoints)
	for k, point in pairs(self.intersectionPoints) do
		print("point")
		self:trimPoint(point)
	end
	self:replaceEntities()
	-- should continue this way
        -- message("Click on the part of the entity to remove")
        
        luaInterface:triggerEvent('operationFinished')
end

function TrimOperation:close()
    if(not self.finished) then
        self.finished = true

        luaInterface:deleteEvent("point", self)
        luaInterface:deleteEvent("selectionChanged", self)

        luaInterface:triggerEvent('operationFinished')
    end
end
