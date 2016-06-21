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

function create_dim_angular()
    op[active_widget().id] = DimAngularOperations(active_widget().id)
end

function create_dim_diametric()
    op[active_widget().id] = DimDiametricOperations(active_widget().id)
end
function create_dim_linear()
    op[active_widget().id] = DimLinearOperations(active_widget().id)
end

function create_dim_radial()
    op[active_widget().id] = DimRadialOperations(active_widget().id)
end