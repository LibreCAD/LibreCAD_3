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

function TrimOperation:_init(id)
    Operations._init(self, id)

    self.limit = nil
    self.toTrim = nil
    self.intersectionPoints = {}
    self.toRemovePoint = nil

    event.register("point", self)
    event.register("selectionChanged", self)

    message("Select limit entity")
end

function TrimOperation:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "selectionChanged") then
        self:selectionChanged()
    elseif(eventName == "point") then
        self:newPoint(...)
    end
end

function TrimOperation:selectionChanged()
    if(self.toTrim == nil) then
        nbEntities = #active_widget():selection()
        if(nbEntities == 1) then
            if(self.limit == nil) then
                self.limit = active_widget():selection()[1]

                message("Select entity to trim")
            elseif(self.toTrim == nil) then
                self.toTrim = active_widget():selection()[1]

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
    local intersect = IntersectMany({self.toTrim, self.limit})
    self.intersectionPoints = intersect:result()

    if(#self.intersectionPoints == 0) then
        message("No intersection found")

        self:close()
    elseif(#self.intersectionPoints >= 1) then
        message("Click on the part of the entity to remove")
    end
end

function TrimOperation:trim()
    local newEntity
    if(self.toTrim.entityType == "line") then
        local point = self.toTrim:nearestPointOnEntity(self.toRemovePoint)

        local startToPoint = point:distanceTo(self.toTrim:start())
        local startToIntersect = point:distanceTo(self.intersectionPoints[1])

        if(startToPoint >= startToIntersect) then
            newEntity = Line(self.toTrim:start(), self.intersectionPoints[1], self.toTrim:layer())
        else
            newEntity = Line(self.intersectionPoints[1], self.toTrim:finish(), self.toTrim:layer())
        end
    end

    local b = Builder(active_widget():document())
    b:append(self.toTrim)

    b:push()
    b:remove()
    b:processStack()

    b:append(newEntity)
    b:execute()

    self:close()
end

function TrimOperation:close()
    if(not self.finished) then
        self.finished = true

        event.delete("point", self)
        event.delete("selectionChanged", self)

        event.trigger('operationFinished')
    end
end