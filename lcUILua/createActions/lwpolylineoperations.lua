LWPolylineOperations = {
    command_line = "POLYLINE",
    icon = "polylines.svg",
    description = "Polyline",
    menu_actions = {
        default = "actionPolyline"
    },
    operation_options = {
        Line = {
            icon = "linesnormal.png",
            action = "createLine"
        },
        Arc = {
            icon = "arc.svg",
            action = "createArc"
        }
    }
}
LWPolylineOperations.__index = LWPolylineOperations

setmetatable(LWPolylineOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function LWPolylineOperations:_init()
    CreateOperations._init(self, lc.builder.LWPolylineBuilder, "enterPoint")
	self.currentVertex = "line"
	self.tempPoint = false
end

function LWPolylineOperations:_init_default()
    message("<b>Polyline</b>")
    message("Click on first point or enter coordinates:")
end

function LWPolylineOperations:enterPoint(eventName, data)
	
	if(self.tempPoint) then
		self.builder:removeVertex(-1)
		self.tempPoint = false
	end

	if(eventName == "mouseMove") then
        self:newData(data["position"])
		self.tempPoint = true
	elseif(eventName == "point") then
        self:newData(data["position"])
	elseif(eventName == "number") then
	self.builder:setWidth(data["number"])
	print(data["number"])
    end
end

function LWPolylineOperations:newData(data)
    if(data ~= nil) then
        if(self.currentVertex == "line") then
			self.builder:addLineVertex(data)
        elseif(self.currentVertex == "arc") then
			self.builder:addArcVertex(data)
        end
    else
		self.builder:modifyLastVertex(data)
    end
end

function LWPolylineOperations:refreshTempEntity()
	if(#self.builder:getVertices() > 1) then
        CreateOperations.refreshTempEntity(self)
    end
end

function LWPolylineOperations:close()
    if(self.tempPoint) then
            self.builder:removeVertex(-1)
            self.tempPoint = false
    end

	if(self.creatingPolyspline == nil) then
        if(#self.builder:getVertices() > 1) then
            self.creatingPolyspline = true
            self:createEntity()
        end
        CreateOperations.close(self)
    end
end

function LWPolylineOperations:createArc()
    self.currentVertex = "arc"

    if(#self.builder:getVertices() > 1) then
		if(self.tempPoint) then
			self.builder:removeVertex(-1)
			self.tempPoint = false
		end

		self.builder:modifyLastVertexArc()
    end

    message("Give arc angle and coordinates")
end

function LWPolylineOperations:createLine()
    self.currentVertex = "line"

    if(#self.builder:getVertices() > 1) then
		if(self.tempPoint) then
			self.builder:removeVertex(-1)
			self.tempPoint = false
		end

		self.builder:modifyLastVertexLine()
    end

    message("Give line coordinates")
end
