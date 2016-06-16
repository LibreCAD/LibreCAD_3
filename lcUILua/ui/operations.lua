function create_line()
    op[active_widget().id] = LineOperations(active_widget().id)
end

function create_circle()
    op[active_widget().id] = CircleOperations(active_widget().id)
end