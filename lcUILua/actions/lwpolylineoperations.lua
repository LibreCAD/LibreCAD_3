LWPolylineOperations = {}
LWPolylineOperations.__index = LWPolylineOperations

setmetatable(LWPolylineOperations, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function LWPolylineOperations:_init(id)
    Operations._init(self, id)

    self.currentVertex_Bulge = 1
    self.currentVertex_StartWidth = 0
    self.currentVertex_EndWidth = 0

    self.lwVertexes = {}
    self.lwPolyline_id = ID():id()
    self.lwPolyline = nil

    event.register('point', self)
    event.register('mouseMove', self)
    event.register('number', self)
    event.register('text', self)

    message("Choose entity type")
end

function LWPolylineOperations:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(...)
    elseif(eventName == "mouseMove") then
        self:createTempLWPolyline(...)
    end
end

function LWPolylineOperations:newData(data)
    local point = Operations:getCoordinate(data)
    if(point ~= nil) then
        if(self.currentVertex == "line") then
            table.insert(self.lwVertexes, LWVertex2D(data))
        elseif(self.currentVertex == "arc") then
            table.insert(self.lwVertexes, LWVertex2D(data, self.currentVertex_Bulge))
        end
    else
        local vertex = self.lwVertexes[#self.lwVertexes]
        
        self.currentVertex_Bulge = math.tan(data / 4)
        self.lwVertexes[#self.lwVertexes] = LWVertex2D(vertex:location(), self.currentVertex_Bulge, vertex:startWidth(), vertex:endWidth())
    end
end

function LWPolylineOperations:getLWPolyline(vertexes)
    if(#vertexes > 1) then
        local d = active_widget():document()
        local layer = active_layer()
        local lwp = LWPolyline(vertexes, 1, 1, 1, false, Coord(0,0), layer, MetaInfo())
        lwp:setId(self.lwPolyline_id)

        return lwp
    else
        return nil
    end
end

function LWPolylineOperations:createTempVertex(point)
    local location = self.currentVertex_Location
    local bulge = self.currentVertex_Bulge

    if(location == nil) then
        location = Operations:getCoordinate(point)
    elseif(bulge == nil) then
        bulge = Operations:getDistance(location, point)
    end

    bulge = bulge or 1

    return LWVertex2D(location, bulge)
end

function LWPolylineOperations:createTempLWPolyline(point)
    if(self.lwPolyline ~= nil) then
        active_widget():tempEntities():removeEntity(self.lwPolyline)
    end

    local vertexes = {}
    for k, v in pairs(self.lwVertexes) do
        vertexes[k] = v
    end
    table.insert(vertexes, self:createTempVertex(point))

    self.lwPolyline = self:getLWPolyline(vertexes)

    if(self.lwPolyline ~= nil) then
        active_widget():tempEntities():addEntity(self.lwPolyline)
    end
end

function LWPolylineOperations:createLWPolyline()
    local b = Builder(active_widget():document())
    local lwp = self:getLWPolyline(self.lwVertexes)
    b:append(lwp)
    b:execute()
end

function LWPolylineOperations:close()
    if(not self.finished) then
        if(self.lwPolyline ~= nil) then
            active_widget():tempEntities():removeEntity(self.lwPolyline)
        end
        self.finished = true

        self:createLWPolyline()

        event.delete('mouseMove', self)
        event.delete('number', self)
        event.delete('point', self)
        event.delete('text', self)

        event.trigger('operationFinished')
    end
end

function LWPolylineOperations:createArc()
    self.currentVertex = "arc"

    if(#self.lwVertexes > 0) then
        local vertex = self.lwVertexes[#self.lwVertexes]
        self.lwVertexes[#self.lwVertexes] = LWVertex2D(vertex:location(), self.currentVertex_Bulge, vertex:startWidth(), vertex:endWidth())
    end

    message("Give arc angle and coordinates")
end

function LWPolylineOperations:createLine()
    self.currentVertex = "line"

    if(#self.lwVertexes > 0) then
        local vertex = self.lwVertexes[#self.lwVertexes]
        self.lwVertexes[#self.lwVertexes] = LWVertex2D(vertex:location(), 0, vertex:startWidth(), vertex:endWidth())
    end

    message("Give line coordinates")
end