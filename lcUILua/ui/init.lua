require 'actions.operations'

op = {}

function new_file()
	cadMdiChild = lc.CadMdiChild()
	cadMdiChild:newDocument()
	window = mdiArea:addSubWindow(cadMdiChild)
	cadMdiChild:showMaximized()
	cadMdiChild:viewer():autoScale()
	cadMdiChild:setDestroyCallback(onMdiChildDestroyed)
	
	luaInterface:luaConnect(cadMdiChild:view(), "mousePressEvent()", click)

	id = nextTableId(op)
	op[id] = Operations:new()
	op[id].id = id
	window:widget().id = id
end

function open_file()
	local fileName = qt.QFileDialog.getOpenFileName(
		mainWindow,
		qt.QObject.tr("Open File",  "", -1),
		qt.QString(""),
		qt.QObject.tr("dxf(*.dxf);;dwg(*.dwg)",  "", -1)
	)

	if(not fileName:isEmpty()) then
		load_file(fileName)
	end
end

function undo()
	local activeSubWindow = mdiArea:activeSubWindow()

	if(activeSubWindow ~= nil) then
		activeSubWindow:widget():undoManager():undo()
	end
end

function redo()
	local activeSubWindow = mdiArea:activeSubWindow()

	if(activeSubWindow ~= nil) then
		activeSubWindow:widget():undoManager():redo()
	end
end

function load_file(fileName)
	cadMdiChild = lc.CadMdiChild()
	cadMdiChild:import(fileName:toStdString())
	window = mdiArea:addSubWindow(cadMdiChild)
	cadMdiChild:showMaximized()
	luaInterface:luaConnect(cadMdiChild:view(), "mousePressEvent()", click)

	id = nextTableId(op)
	op[id] = Operations:new()
	op[id].id = id
	window:widget().id = id
end

function click()
	local widget = mdiArea:activeSubWindow():widget()
	local position = widget:cursor():position()
	local x = position:x()
	local y = position:y()

	event.trigger('point', {x, y})
end

function create_line()
	local widget = mdiArea:activeSubWindow():widget()
	op[widget.id]:create_line()
end

function onMdiChildDestroyed(id)
	op[id] = nil
end

function nextTableId(table)
	count = 0
	for id, v in pairs(table) do
		count = count + 1
		if(v == nil) then
			return count
		end
	end

	return count
end

--UI
mainWindow = qt.loadUi(ui_path .. "/mainwindow.ui")
mainWindow:setWindowTitle(qt.QObject.tr("LibreCAD", "", -1)) -- Todo: optional arguments
mainWindow:setUnifiedTitleAndToolBarOnMac(true)

menuBar = mainWindow:menuBar()
drawMenu = menuBar:addMenuStr(qt.QString("Draw"))
lineAction = drawMenu:addActionStr(qt.QString("Line"))

mdiArea = mainWindow:findChild("centralWidget"):findChild("mdiArea")

mdiArea:setHorizontalScrollBarPolicy(0)
mdiArea:setVerticalScrollBarPolicy(0)

mainWindow:setCentralWidget(mdiArea)

new_file()

luaInterface:luaConnect(mainWindow:findChild("actionNew"), "triggered(bool)", new_file)
luaInterface:luaConnect(mainWindow:findChild("actionOpen"), "triggered(bool)", open_file)
luaInterface:luaConnect(mainWindow:findChild("actionUndo"), "triggered(bool)", undo)
luaInterface:luaConnect(mainWindow:findChild("actionRedo"), "triggered(bool)", redo)
luaInterface:luaConnect(lineAction, "triggered(bool)", create_line)

luaScript = lc.LuaScript(mdiArea)
mainWindow:addDockWidget(2, luaScript)

mainWindow:show();