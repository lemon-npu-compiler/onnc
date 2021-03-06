//===- DigraphTest.h ------------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <skypat/skypat.h>
#include <onnc/ADT/Digraph.h>
#include <onnc/ADT/NodeIterator.h>
#include <onnc/ADT/ArcIterator.h>
#include <onnc/ADT/TypeTraits.h>
#include <onnc/Support/IOStream.h>
#include <onnc/ADT/TopologyIterator.h>

using namespace skypat;
using namespace onnc;

//===----------------------------------------------------------------------===//
// DigraphTest
//===----------------------------------------------------------------------===//
namespace {

struct MyNode;
struct MyArc;

struct MyNode : public onnc::DigraphNode<MyNode, MyArc>
{
  MyNode(int pData) : data(pData) { }

  int data;
};

struct MyArc : public onnc::DigraphArc<MyNode, MyArc>
{
  MyArc(int pData) : value(pData) {}

  int value;
};

typedef Digraph<MyNode, MyArc> MyGraph;

} // anonymous namespace

SKYPAT_F(DigraphTest, trivial)
{
  MyGraph g;
  MyNode* n1 = g.addNode(5);
  MyNode* n2 = g.addNode(4);

  MyArc* a = g.connect(*n1, *n2, 3);

  ASSERT_EQ(n1->data, a->getSource()->data);
  ASSERT_EQ(n2->data, a->getTarget()->data);
  ASSERT_EQ(g.getNodeSize(), 2);
  ASSERT_EQ(g.getArcSize(), 1);
}

SKYPAT_F(DigraphTest, clear)
{
  MyGraph g;
  MyNode* n1 = g.addNode(5);
  MyNode* n2 = g.addNode(4);

  MyArc* a = g.connect(*n1, *n2, 3);

  ASSERT_EQ(n1->data, a->getSource()->data);
  ASSERT_EQ(n2->data, a->getTarget()->data);
  ASSERT_EQ(g.getNodeSize(), 2);
  ASSERT_EQ(g.getArcSize(), 1);

  g.clear();
  ASSERT_EQ(g.getNodeSize(), 0);
  ASSERT_EQ(g.getArcSize(), 0);
}

SKYPAT_F(DigraphTest, node_iterator)
{
  MyGraph g;
  MyNode* n1 = g.addNode(5);
  MyNode* n2 = g.addNode(4);
  g.addNode(3);
  g.addNode(2);
  g.connect(*n1, *n2, 3);

  MyGraph::iterator iter = g.begin();
  ASSERT_EQ(iter->data, 5);
  ASSERT_TRUE(iter.hasNext());
  ++iter;
  ASSERT_EQ(iter->data, 4);
  ASSERT_TRUE(iter.hasNext());
  ++iter;
  ASSERT_EQ(iter->data, 3);
  ASSERT_TRUE(iter.hasNext());
  ++iter;
  ASSERT_EQ(iter->data, 2);
  ASSERT_FALSE(iter.hasNext());
  ++iter;
  ASSERT_TRUE(iter == g.end());
}

SKYPAT_F(DigraphTest, node_erase_head)
{
  MyGraph g;
  MyNode* n1 = g.addNode(1);
  MyNode* n2 = g.addNode(2);
  g.addNode(3);
  g.addNode(4);
  g.connect(*n1, *n2, 1);
  g.erase(*n1);

  NodeIterator<MyNode, NonConstTraits> iter(n1);
  // n1 became a free node. g shall not delete n1 until its destrunction.
  ASSERT_EQ(n1->data, 1);
  ASSERT_FALSE(iter.hasNext());
}

SKYPAT_F(DigraphTest, node_erase_middle)
{
  MyGraph g;
  MyNode* n1 = g.addNode(1);
  MyNode* n2 = g.addNode(2);
  g.addNode(3);
  g.addNode(4);
  g.connect(*n1, *n2, 3);

  g.erase(*n2);

  MyGraph::iterator iter = g.begin();
  ASSERT_EQ(iter->data, 1);
  ASSERT_TRUE(iter.hasNext());
  ++iter;
  ASSERT_EQ(iter->data, 3);
  ASSERT_TRUE(iter.hasNext());
  ++iter;
  ASSERT_EQ(iter->data, 4);
  ASSERT_FALSE(iter.hasNext());
}

SKYPAT_F(DigraphTest, node_erase_rear)
{
  MyGraph g;
  MyNode* n1 = g.addNode(1);
  MyNode* n2 = g.addNode(2);
  g.addNode(3);
  MyNode* n4 = g.addNode(4);
  g.connect(*n1, *n2, 3);

  g.erase(*n4);

  NodeIterator<MyNode, NonConstTraits> iter(n1);
  ASSERT_EQ(iter->data, 1);
  ASSERT_TRUE(iter.hasNext());
  ++iter;
  ASSERT_EQ(iter->data, 2);
  ASSERT_TRUE(iter.hasNext());
  ++iter;
  ASSERT_EQ(iter->data, 3);
  ASSERT_FALSE(iter.hasNext());
}

SKYPAT_F(DigraphTest, edge_iterator)
{
  MyGraph g;
  MyNode* n1 = g.addNode(1);
  MyNode* n2 = g.addNode(2);
  MyNode* n3 = g.addNode(3);
  MyNode* n4 = g.addNode(4);
  MyNode* n5 = g.addNode(5);
  g.connect(*n1, *n2, 1);
  g.connect(*n1, *n3, 2);
  g.connect(*n3, *n2, 3);
  g.connect(*n2, *n4, 4);
  g.connect(*n3, *n5, 5);

  InArcIterator<MyNode, MyArc> iter(*n2);
  ASSERT_EQ(iter->value, 1);
  ASSERT_TRUE(iter.hasNext());
  ++iter;
  ASSERT_EQ(iter->value, 3);
  ASSERT_FALSE(iter.hasNext());
  ++iter;
  ASSERT_TRUE(iter.isEnd());

  OutArcIterator<MyNode, MyArc> out(*n1);
  ASSERT_EQ(out->value, 1);
  ASSERT_TRUE(out.hasNext());
  ++out;
  ASSERT_EQ(out->value, 2);
  ASSERT_FALSE(out.hasNext());
  ++out;
  ASSERT_TRUE(out.isEnd());

  OutArcIterator<MyNode, MyArc> out2(*n3);
  ASSERT_EQ(out2->value, 3);
  ASSERT_TRUE(out2.hasNext());
  ++out2;
  ASSERT_EQ(out2->value, 5);
  ASSERT_FALSE(out2.hasNext());
  ++out2;
  ASSERT_TRUE(out2.isEnd());
}

SKYPAT_F(DigraphTest, bfs_iterator)
{
  MyGraph g;
  MyNode* n1 = g.addNode(1);
  MyNode* n2 = g.addNode(2);
  MyNode* n3 = g.addNode(3);
  MyNode* n4 = g.addNode(4);
  MyNode* n5 = g.addNode(5);
  g.connect(*n1, *n2, 1);
  g.connect(*n1, *n3, 2);
  g.connect(*n3, *n2, 3);
  g.connect(*n2, *n4, 4);
  g.connect(*n3, *n5, 5);

  MyGraph::bfs_iterator iter = g.bfs_begin();
  ASSERT_EQ(iter->data, 1);
  iter.next();
  ASSERT_EQ(iter->data, 3);
  iter.next();
  ASSERT_EQ(iter->data, 2);
  iter.next();
  ASSERT_EQ(iter->data, 5);
  iter.next();
  ASSERT_EQ(iter->data, 4);
  iter.next();
  ASSERT_TRUE(g.bfs_end() == iter);
}

SKYPAT_F(DigraphTest, dfs_iterator)
{
  MyGraph g;
  MyNode* n1 = g.addNode(1);
  MyNode* n2 = g.addNode(2);
  MyNode* n3 = g.addNode(3);
  MyNode* n4 = g.addNode(4);
  MyNode* n5 = g.addNode(5);
  g.connect(*n1, *n2, 1);
  g.connect(*n1, *n3, 2);
  g.connect(*n3, *n2, 3);
  g.connect(*n2, *n4, 4);
  g.connect(*n3, *n5, 5);

  MyGraph::dfs_iterator iter = g.dfs_begin();
  ASSERT_EQ(iter->data, 1);
  iter.next();
  ASSERT_EQ(iter->data, 3);
  iter.next();
  ASSERT_EQ(iter->data, 2);
  iter.next();
  ASSERT_EQ(iter->data, 4);
  iter.next();
  ASSERT_EQ(iter->data, 5);
  iter.next();
  ASSERT_TRUE(g.dfs_end() == iter);
}

SKYPAT_F(DigraphTest, default_graph)
{
  Digraph<> g;
  digraph::NodeBase* n1 = g.addNode();
  digraph::NodeBase* n2 = g.addNode();
  digraph::NodeBase* n3 = g.addNode();
  digraph::NodeBase* n4 = g.addNode();
  digraph::NodeBase* n5 = g.addNode();
  g.connect(*n1, *n2);
  g.connect(*n1, *n3);
  g.connect(*n3, *n2);
  g.connect(*n2, *n4);
  g.connect(*n3, *n5);

  Digraph<>::dfs_iterator iter = g.dfs_begin();
  ASSERT_TRUE(&*iter == n1);
  iter.next();
  ASSERT_TRUE(&*iter == n3);
  iter.next();
  ASSERT_TRUE(&*iter == n2);
  iter.next();
  ASSERT_TRUE(&*iter == n4);
  iter.next();
  ASSERT_TRUE(&*iter == n5);
  iter.next();
  ASSERT_TRUE(g.dfs_end() == iter);
}

SKYPAT_F(DigraphTest, topology_iterator_test_1)
{
  Digraph<> g;
  digraph::NodeBase* n1 = g.addNode();
  digraph::NodeBase* n2 = g.addNode();
  digraph::NodeBase* n3 = g.addNode();
  digraph::NodeBase* n4 = g.addNode();
  digraph::NodeBase* n5 = g.addNode();
  g.connect(*n1, *n2);
  g.connect(*n1, *n3);
  g.connect(*n2, *n3);
  g.connect(*n2, *n4);
  g.connect(*n3, *n5);

  using Iterator = TopologyIterator<TopoDFSAlgo, digraph::NodeBase, NonConstTraits>;
  Iterator iter(n1);

  ASSERT_TRUE(&*iter == n1);
  iter.next();
  ASSERT_TRUE(&*iter == n2);
  iter.next();
  ASSERT_TRUE(&*iter == n4);
  iter.next();
  ASSERT_TRUE(&*iter == n3);
  iter.next();
  ASSERT_TRUE(&*iter == n5);
  iter.next();
  ASSERT_TRUE(iter.isEnd());
}

SKYPAT_F(DigraphTest, topology_iterator_test_2)
{
  Digraph<> g;
  digraph::NodeBase* n1 = g.addNode();
  digraph::NodeBase* n2 = g.addNode();
  digraph::NodeBase* n3 = g.addNode();
  digraph::NodeBase* n4 = g.addNode();
  digraph::NodeBase* n5 = g.addNode();
  g.connect(*n1, *n2);
  g.connect(*n1, *n3);
  g.connect(*n3, *n2);
  g.connect(*n2, *n4);
  g.connect(*n3, *n5);

  using Iterator = TopologyIterator<TopoDFSAlgo, digraph::NodeBase, NonConstTraits>;
  Iterator iter(n1);

  ASSERT_TRUE(&*iter == n1);
  iter.next();
  ASSERT_TRUE(&*iter == n3);
  iter.next();
  ASSERT_TRUE(&*iter == n2);
  iter.next();
  ASSERT_TRUE(&*iter == n4);
  iter.next();
  ASSERT_TRUE(&*iter == n5);
  iter.next();
  ASSERT_TRUE(iter.isEnd());
}

SKYPAT_F(DigraphTest, topology_iterator_test_3)
{
  Digraph<> g;
  digraph::NodeBase* n1 = g.addNode();
  digraph::NodeBase* n2 = g.addNode();
  digraph::NodeBase* n3 = g.addNode();
  digraph::NodeBase* n4 = g.addNode();
  digraph::NodeBase* n5 = g.addNode();
  digraph::NodeBase* n6 = g.addNode();
  digraph::NodeBase* n7 = g.addNode();
  digraph::NodeBase* n8 = g.addNode();

  g.connect(*n1, *n2);
  g.connect(*n1, *n3);
  g.connect(*n1, *n4);
  g.connect(*n2, *n5);
  g.connect(*n3, *n5);
  g.connect(*n3, *n6);
  g.connect(*n4, *n6);
  g.connect(*n5, *n7);
  g.connect(*n6, *n7);
  g.connect(*n6, *n8);

  errs() << std::hex << (void*)n1 << " ";
  errs() << std::hex << (void*)n2 << " ";
  errs() << std::hex << (void*)n3 << " ";
  errs() << std::hex << (void*)n4 << " ";
  errs() << std::hex << (void*)n5 << " ";
  errs() << std::hex << (void*)n6 << " ";
  errs() << std::hex << (void*)n7 << " ";
  errs() << std::hex << (void*)n8 << std::endl;

  using Iterator = TopologyIterator<RevTopoDFSAlgo, digraph::NodeBase, NonConstTraits>;
  Iterator iter(n7);

  ASSERT_TRUE(&*iter == n7);
  iter.next();
  ASSERT_TRUE(&*iter == n5);
  iter.next();
  ASSERT_TRUE(&*iter == n2);
  iter.next();
  ASSERT_TRUE(&*iter == n6);
  iter.next();
  ASSERT_TRUE(&*iter == n3);
  iter.next();
  ASSERT_TRUE(&*iter == n4);
  iter.next();
  ASSERT_TRUE(&*iter == n1);
  iter.next();
  ASSERT_TRUE(iter.isEnd());
}

SKYPAT_F(DigraphTest, topology_iterator_test_4)
{
  Digraph<MyNode> g;
  MyNode* n1 = g.addNode(0);
  MyNode* n2 = g.addNode(1);
  MyNode* n3 = g.addNode(2);
  MyNode* n4 = g.addNode(3);
  MyNode* n5 = g.addNode(4);
  MyNode* n6 = g.addNode(5);
  MyNode* n7 = g.addNode(6);
  MyNode* n8 = g.addNode(7);

  g.connect(*n1, *n2);
  g.connect(*n1, *n3);
  g.connect(*n1, *n4);
  g.connect(*n2, *n5);
  g.connect(*n3, *n5);
  g.connect(*n3, *n6);
  g.connect(*n4, *n6);
  g.connect(*n5, *n7);
  g.connect(*n6, *n7);
  g.connect(*n6, *n8);

  using Iterator = TopologyIterator<RevTopoDFSAlgo, MyNode, NonConstTraits>;
  Iterator iter(n7);

  while (!iter.isEnd()) {
    errs() << iter->data << " ";
    iter.next();
  }
  errs() << std::endl;

  Iterator iter2(n6);
  while (!iter2.isEnd()) {
    errs() << iter2->data << " ";
    iter2.next();
  }
  errs() << std::endl;
}
