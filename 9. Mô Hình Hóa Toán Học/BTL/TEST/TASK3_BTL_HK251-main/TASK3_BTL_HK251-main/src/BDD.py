import collections
from typing import Tuple, List, Optional
from pyeda.inter import *
from .PetriNet import PetriNet
from collections import deque
import numpy as np

def bdd_reachable(pn: PetriNet) -> Tuple[BinaryDecisionDiagram, int]:
    # TODO trả về BinaryDecisionDiagram và số lượng reachable makes ()