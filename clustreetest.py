from clustree import Clustree
import unittest
import numpy as np

import copy as cp

def is_equal(a,b):
    return np.alltrue(a == b)

class ClustreeTest(unittest.TestCase):

    def setUp(self):
        self.elem1 = np.array([1.0,2.0,3.0])
        self.elem2 = np.array([2.0,3.0,4.0])
        self.elem4 = np.array([3.0,5.0,5.0])
        self.elem3 = np.array([0.0,-1.0,2.0])

        self.ctree = Clustree(node_max_size=2, max_distance=3.0)

    def test_create_and_add_elem(self):
        
        self.ctree.add_elem(self.elem1)

        self.assertEqual(len(self.ctree.root.cfs), 1)

        np.testing.assert_array_equal(self.ctree.root.cfs[0].ls, self.elem1)
        np.testing.assert_array_equal(self.ctree.root.cfs[0].ss, self.elem1*self.elem1)
        self.assertEqual(self.ctree.root.cfs[0].n, 1)
        self.assertEqual(len(self.ctree.root.childs), 0)

    def test_add_two_elem_effect_merge(self):
        self.ctree.add_elem(self.elem1)
        self.ctree.add_elem(self.elem2)

        self.assertEqual(len( self.ctree.root.cfs ), 1 )
        self.assertEqual(len( self.ctree.root.childs ), 0 )

        np.testing.assert_array_equal(self.ctree.root.cfs[0].ls, self.elem1+self.elem2 )
        np.testing.assert_array_equal(self.ctree.root.cfs[0].ss, self.elem1*self.elem1 + self.elem2*self.elem2)
        self.assertEqual(self.ctree.root.cfs[0].n, 2)

    def test_add_two_elem_effect_two_cfs(self):

        self.ctree.add_elem(self.elem1)
        self.ctree.add_elem(self.elem4)

        self.assertEqual(len(self.ctree.root.cfs), 2)
        self.assertEqual(len(self.ctree.root.childs), 0)

        np.testing.assert_array_equal(self.ctree.root.cfs[0].ls, self.elem1)
        np.testing.assert_array_equal(self.ctree.root.cfs[0].ss, self.elem1*self.elem1)
        self.assertEqual(self.ctree.root.cfs[0].n, 1)

        np.testing.assert_array_equal(self.ctree.root.cfs[1].ls, self.elem4)
        np.testing.assert_array_equal(self.ctree.root.cfs[1].ss, self.elem4*self.elem4)
        self.assertEqual(self.ctree.root.cfs[1].n, 1)

    def test_add_three_elem_effect_new_leafs(self):

        self.ctree.add_elem(self.elem1)
        self.ctree.add_elem(self.elem4)
        self.ctree.add_elem(self.elem3)

        self.assertEqual(len(self.ctree.root.cfs), 2)
        self.assertEqual(len(self.ctree.root.childs), 2)

        np.testing.assert_array_equal(self.ctree.root.cfs[0].ls, self.elem1 + self.elem3)
        np.testing.assert_array_equal(self.ctree.root.cfs[0].ss, self.elem1*self.elem1 + self.elem3*self.elem3 )
        self.assertEqual(self.ctree.root.cfs[0].n, 2)

        np.testing.assert_array_equal(self.ctree.root.childs[0].cfs[0].ls, self.elem1)
        np.testing.assert_array_equal(self.ctree.root.childs[0].cfs[0].ss, self.elem1*self.elem1)
        self.assertEqual( self.ctree.root.childs[0].cfs[0].n, 1)

        np.testing.assert_array_equal(self.ctree.root.childs[0].cfs[1].ls, self.elem3)
        np.testing.assert_array_equal(self.ctree.root.childs[0].cfs[1].ss, self.elem3*self.elem3)
        self.assertEqual(self.ctree.root.childs[0].cfs[1].n, 1)

if __name__=='__main__':
    unittest.main()
