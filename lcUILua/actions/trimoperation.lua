--TODO: may not work, it should be moved to C++

TrimOperation = {}
TrimOperation.__index = TrimOperation

setmetatable(TrimOperation, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function TrimOperation:init()
    mainWindow:getToolbar():addButton("", ":/icons/modifytrim.png", "Modify", 2, 1, function() run_basic_operation(TrimOperation) end, "Trim")

    mainWindow:getCliCommand():addCommand("TRIM", function() run_basic_operation(TrimOperation) end)
end

function TrimOperation:_init(id)
    Operations._init(self)

    self.limit = nil
    self.toTrim = nil
    self.intersectionPoints = {}
    self.toRemovePoint = nil

    luaInterface:registerEvent("point", self)
    luaInterface:registerEvent("selectionChanged", self)

    message("Select limit entity")
end

function TrimOperation:onEvent(eventName, data)
    if(eventName == "selectionChanged") then
        self:selectionChanged()
    elseif(eventName == "point") then
        self:newPoint(data)
    end
end

function TrimOperation:selectionChanged()
    if(self.toTrim == nil) then
        local window = mainWindow:getCadMdiChild()
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

function TrimOperation:newPoint(point)
    if(#self.intersectionPoints >= 1) then
        self.toRemovePoint = Operations:getCoordinate(point)

        self:trim()
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
    local b = lc.operation.EntityBuilder(mainWindow:getCadMdiChild():document())
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
