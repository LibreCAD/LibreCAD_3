gridSnapableOptions = {}
gridSnapableOptions.__index = gridSnapableOptions

setmetatable(gridSnapableOptions, {
    __index = optionsOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function gridSnapableOptions:_init(enabled)
    active_widget():getSnapManager():setGridSnappable(enabled)
end