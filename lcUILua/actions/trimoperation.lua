TrimOperation = {
    command_line = "TRIM",
    icon = "modifytrim.png"
}
TrimOperation.__index = TrimOperation

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

    if(#self.toTrim > 0) then
        luaInterface:registerEvent('point', self)
	luaInterface:registerEvent("selectionChanged", self)
        message("Select limit entity or Give split point")
    else
        self.finished = true
        message('select some entities to trim')
        luaInterface:triggerEvent('operationFinished')
    end
end

function TrimOperation:onEvent(eventName, data)
    if(eventName == "selectionChanged") then
        self:selectionChanged()
    elseif(eventName == "point") then
        self:newPoint(data["position"])
    end
end

-- return newEntities based on trim point
function TrimOperation:trimPoint(point)
	print(#self.toTrim)
	for k, entity in pairs(self.toTrim) do
		local entities = lc.entity.Splitable.splitHelper(entity, point)
		    if(#entities > 0) then
		    	--split sucessful
		    	print('split on main entity')
			for k2, entity2 in pairs(entities) do
				table.insert(self.newEntities, entity2)
			end
			table.remove(self.toTrim, k)
			table.insert(self.oldEntities, entity) -- remove old
	    	    end
	end
	-- if it splits new created entity
--	for k, entity in pairs(self.newEntities) do
--		local entities = lc.entity.Splitable.splitHelper(self.entity, point)
--		    if(#entities > 0) then
--		    	--split sucessful
--		    	print('split on new entity')
--			for k2, entity2 in pairs(self.toTrim) do
--				table.insert(self.newEntities, entity2)
--			end
--			table.remove(self.newEntity, entity) -- we do not need it
--	    	    end
--	end
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
    if(self.toTrim == nil) then
        local window = mainWindow:cadMdiChild()
        local nbEntities = #window:selection()
        if(nbEntities == 1) then
            if(self.limit == nil) then
                self.limit = window:selection()[1]

                message("Select entity to trim")
            elseif(self.toTrim == nil) then
                self.toTrim = window:selection()[1]

                self:getIntersectionPoints()
            end
        elseif(nbEntities > 1) then
            message("Select only one entity")
        end
    end
end

function TrimOperation:getIntersectionPoints()
    local intersect = lc.maths.IntersectMany({self.toTrim, self.limit})
    self.intersectionPoints = intersect:result()

    if(#self.intersectionPoints == 0) then
        message("No intersection found")

        self:close()
    elseif(#self.intersectionPoints >= 1) then
        message("Click on the part of the entity to remove")
    end
end

function TrimOperation:trim()
    local b = lc.operation.EntityBuilder(mainWindow:cadMdiChild():document())
    b:appendEntity(self.toTrim)

    b:appendOperation(lc.operation.Push())
    b:appendOperation(lc.operation.Remove())
    b:processStack()

    if(self.toTrim.entityType == "line") then
        local point = self.toTrim:nearestPointOnEntity(self.toRemovePoint)
        local start = self.toTrim:start()
        local finish = self.toTrim:finish()

        local startToPoint = point:distanceTo(start)

        local previousIntersect = start
        local previousIntersectDistance = 0

        local nextIntersect = finish
        local nextIntersectDistance = start:distanceTo(finish)

        for k, v in pairs(self.intersectionPoints) do
            local startToIntersect = start:distanceTo(v)

            if(startToIntersect < startToPoint and startToIntersect > previousIntersectDistance) then
                previousIntersect = v
                previousIntersectDistance = startToIntersect
            elseif(startToIntersect > startToPoint and startToIntersect < nextIntersectDistance) then
                nextIntersect = v
                nextIntersectDistance = startToIntersect
            end
        end

        if(previousIntersect == start) then
            b:appendEntity(Line(nextIntersect, finish, self.toTrim:layer()))
        elseif(nextIntersect == finish) then
            b:appendEntity(Line(start, previousIntersect, self.toTrim:layer()))
        else
            b:appendEntity(Line(start, previousIntersect, self.toTrim:layer()))
            b:appendEntity(Line(nextIntersect, finish, self.toTrim:layer()))
        end
    elseif(self.toTrim.entityType == "circle") then
        if(#self.intersectionPoints < 2) then
            message("Circle need at least two intersection points")

            self:close()
            return
        end

        local center = self.toTrim:center()
        local selectedPoint = self.toTrim:nearestPointOnEntity(self.toRemovePoint)
        local selectedAngle = center:angleTo(selectedPoint)

        local intersectAngles = {}
        local previousAngle
        local nextAngle

        for k, v in pairs(self.intersectionPoints) do
            table.insert(intersectAngles, center:angleTo(v))
        end
        table.sort(intersectAngles)

        for i, v in ipairs(intersectAngles) do
            if(selectedAngle < v) then
                if(i > 1) then
                    previousAngle = intersectAngles[i - 1]
                    nextAngle = v
                    break
                else
                    previousAngle = v
                    nextAngle = intersectAngles[#intersectAngles]
                    break
                end
            elseif(i == #intersectAngles) then
                previousAngle = v
                nextAngle = intersectAngles[1]
                break
            end
        end

        b:appendEntity(lc.entity.Arc(center, self.toTrim:radius(), previousAngle, nextAngle, false, self.toTrim:layer()))

    elseif(self.toTrim.entityType == "arc") then
        local center = self.toTrim:center()
        local startAngle
        local endAngle

        if(self.toTrim:CCW()) then
            startAngle = self.toTrim:startAngle()
            endAngle = self.toTrim:endAngle()
        else
            startAngle = self.toTrim:endAngle()
            endAngle = self.toTrim:startAngle()
        end

        local selectedAngle = center:angleTo(self.toRemovePoint)
        local intersectAngles = {startAngle, endAngle}
        local previousAngle
        local nextAngle

        for k, v in pairs(self.intersectionPoints) do
            table.insert(intersectAngles, center:angleTo(v))
        end
        table.sort(intersectAngles)

        for i, v in ipairs(intersectAngles) do
            if(selectedAngle < v) then
                if(i > 1) then
                    message("first")
                    previousAngle = intersectAngles[i - 1]
                    nextAngle = v
                    break
                else
                    message("second")
                    previousAngle = v
                    nextAngle = intersectAngles[#intersectAngles]
                    break
                end
            elseif(i == #intersectAngles) then
                message("third")
                previousAngle = v
                nextAngle = intersectAngles[1]
                break
            end
        end

        if(nextIntersectAngle ~= endAngle) then
            b:appendEntity(lc.entity.Arc(center, self.toTrim:radius(), nextAngle, endAngle, true, self.toTrim:layer()))
        end
        if(previousIntersectAngle ~= startAngle) then
            b:appendEntity(lc.entity.Arc(center, self.toTrim:radius(), startAngle, previousAngle, true, self.toTrim:layer()))
        end
    else
        message("Unsupported entity")
    end

    b:execute()

    self:close()
end

function TrimOperation:close()
    if(not self.finished) then
        self.finished = true

        luaInterface:deleteEvent("point", self)
        luaInterface:deleteEvent("selectionChanged", self)

        luaInterface:triggerEvent('operationFinished')
    end
end
