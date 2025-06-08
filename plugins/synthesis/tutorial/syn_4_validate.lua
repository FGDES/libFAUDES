
-- This script runs a number of test cases for
-- the purpose of validation 


-- ---------------------------------------------
-- ---------------------------------------------
--
-- SupNormClosed test cases
-- data kindly provided by Tomas Masopust.
--
-- ---------------------------------------------
-- ---------------------------------------------


-- Test case 1

L1=faudes.System("data/syn_supnorm_l1.gen")
So1=faudes.Alphabet("data/syn_supnorm_o1.txt")
K1=faudes.System("data/syn_supnorm_k1.gen")
supN1 = faudes.System()
faudes.SupNormClosed(L1,So1,K1,supN1)

FAUDES_TEST_DUMP("supnorm 1",supN1)


-- Test case 2

L2=faudes.System("data/syn_supnorm_l2.gen")
So2=faudes.Alphabet("data/syn_supnorm_o2.txt")
K2=faudes.System("data/syn_supnorm_k2.gen")
supN2 = faudes.System()
faudes.SupNormClosed(L2,So2,K2,supN2)

FAUDES_TEST_DUMP("supnorm 2",supN2)


-- Test case 3

L3=faudes.System("data/syn_supnorm_l3.gen")
So3=faudes.Alphabet("data/syn_supnorm_o3.txt")
K3=faudes.System("data/syn_supnorm_k3.gen")
supN3 = faudes.System()
faudes.SupNormClosed(L3,So3,K3,supN3)

FAUDES_TEST_DUMP("supnorm 3",supN3)


-- ---------------------------------------------
-- ---------------------------------------------
--
-- SupRelativelyClosed test cases
--
-- ---------------------------------------------
-- ---------------------------------------------

-- Test case 1

L=faudes.Generator("data/syn_suprpc_l.gen")
E=faudes.Generator("data/syn_suprpc_e.gen")
supK = faudes.Generator()
faudes.SupRelativelyClosed(L,E,supK)
L:Write("tmp_syn_suprpc_l.gen")
E:Write("tmp_syn_suprpc_e.gen")
supK:Write("tmp_syn_suprpc_k.gen")

FAUDES_TEST_DUMP("suprpc 1",supK)

-- Validate
FAUDES_TEST_DIFF()