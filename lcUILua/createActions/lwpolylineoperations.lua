LWPolylineOperations = {}
LWPolylineOperations.__index = LWPolylineOperations

setmetatable(LWPolylineOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function LWPolylineOperations:_init(id)
    CreateOperations._init(self, id, lc.builder.LWPolylineBuilder, "enterPoint")
	self.currentVertex = "line"
	self.tempPoint = false
end

function LWPolylineOperations:enterPoint(eventName, data)
	if(eventName == "point" or eventName == "number") then
        self:newData(data["position"])
    elseif(eventName == "mouseMove") then
		--self.builder:createTempLWPolyline(data["position"])
		self:refreshTempEntity()
    end
end

function LWPolylineOperations:newData(data)
    local point = Operations:getCoordinate(data)
    if(point ~= nil) then
        if(self.currentVertex == "line") then
			self.builder:addLineVertex(data)
        elseif(self.currentVertex == "arc") then
			self.builder:addArcVertex(data)
        end
    else
		--self.build:modifyLastVertex(data)
    end
end

--[[function LWPolylineOperations:createLWPolyline()
	self:createEntity()
end]]--

function LWPolylineOperations:refreshTempEntity()
	if(#self.builder:getVertices() > 2) then
        CreateOperations.refreshTempEntity(self)
    end
end

function LWPolylineOperations:close()
	--self:createEntity()
	CreateOperations.close(self)
end

--[[function LWPolylineOperations:createArc()
    self.currentVertex = "arc"

    if(self.builder:getVertices() > 0) then
		self.builder:modifyLastVertexArc()
    end

    message("Give arc angle and coordinates", self.target_widget)
end

function LWPolylineOperations:createLine()
    self.currentVertex = "line"

    if(self.builder:getVertices() > 0) then
		self.builder:modifyLastVertexLine()
    end

    message("Give line coordinates", self.target_widget)
end ]]--