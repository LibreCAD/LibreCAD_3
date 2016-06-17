function create_line()
    op[active_widget().id] = LineOperations(active_widget().id)
end

function create_circle()
    op[active_widget().id] = CircleOperations(active_widget().id)
end

function create_arc()
    op[active_widget().id] = ArcOperations(active_widget().id)
end

function create_ellipse()
    op[active_widget().id] = EllipseOperations(active_widget().id, false)
end

function create_arc_ellipse()
    op[active_widget().id] = EllipseOperations(active_widget().id, true)
end

function create_dim_aligned()
    op[active_widget().id] = DimAlignedOperations(active_widget().id)
end