import os
from PolyRound.api import PolyRoundApi
from PolyRound.static_classes.lp_utils import ChebyshevFinder
from PolyRound.settings import PolyRoundSettings
from pathlib import Path

model_path = os.path.join("PolyRound", "models", "e_coli_core.xml")
# Create a settings object with the default settings.
settings = PolyRoundSettings()
# Import model and create Polytope object
polytope = PolyRoundApi.sbml_to_polytope(model_path)
# Remove redundant constraints and refunction inequality constraints that are de-facto equalities.
# Due to these inequalities, the polytope is empty (distance from chebyshev center to boundary is zero)
x, dist = ChebyshevFinder.chebyshev_center(polytope, settings)
print(dist)
simplified_polytope = PolyRoundApi.simplify_polytope(polytope)
# The simplified polytope has non-zero border distance
x, dist = ChebyshevFinder.chebyshev_center(simplified_polytope, settings)
print(dist)
# Embed the polytope in a space where it has non-zero volume
transformed_polytope = PolyRoundApi.transform_polytope(simplified_polytope)
# The distance from the chebyshev center to the boundary changes in the new coordinate system
x, dist = ChebyshevFinder.chebyshev_center(transformed_polytope, settings)
print(dist)
# Round the polytope
rounded_polytope = PolyRoundApi.round_polytope(transformed_polytope)
# After rounding, the distance from the chebyshev center to the boundary is set to be close to 1
x, dist = ChebyshevFinder.chebyshev_center(rounded_polytope, settings)
print(dist)

# The chebyshev center can be back transformed into an interior point in the simplified space.
print(simplified_polytope.border_distance(rounded_polytope.back_transform(x)))
# simplify, transform and round in one call
one_step_rounded_polytope = PolyRoundApi.simplify_transform_and_round(polytope)
# save to csv
out_csv_dir = os.path.join("PolyRound", "output", "e_coli_core")
Path(out_csv_dir).mkdir(parents=True, exist_ok=True)
PolyRoundApi.polytope_to_csvs(one_step_rounded_polytope, out_csv_dir)
# Special use case: remove redundant constraints without removing zero facettes. This will leave th polytope with its original border distance.
x, dist = ChebyshevFinder.chebyshev_center(polytope, settings)
print(dist)
settings.simplify_only = True
simplified_polytope = PolyRoundApi.simplify_polytope(polytope, settings=settings)
# The simplified polytope still has zero border distance
x, dist = ChebyshevFinder.chebyshev_center(simplified_polytope, settings)
print(dist)
