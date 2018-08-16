#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <list>

enum EditType { SKIP = 0, DELETE = 1, INSERT = 2, REPLACE = 3 };

typedef std::array<unsigned, 4> EditCosts;

EditCosts std_edit_costs { 1, 10, 10, 2 };
const char *edit_type_names[4] = { "SKP", "DEL", "INS", "REP" };

struct Edit
{
  Edit (EditType _type, char _ch) : type (_type), ch (_ch) { }
  EditType type;
  char ch;
};

std::list<Edit>
compute_optimal_edits (const std::string &from, const std::string &to, const EditCosts &costs)
{
  struct EditNode
  {
    EditNode () : edit (SKIP, 0), cost (0) { }
    EditNode (EditType type, char ch, unsigned _cost) : edit (type, ch), cost (_cost) { }

    Edit edit;
    unsigned cost;
  };

  unsigned from_length = from.length ();
  unsigned to_length = to.length ();

  // We calculate the cost matrix a row at a time, but need to keep
  // the entire matrix in memory so we can replay the optimal path at
  // the end.
  //
  // The dimensions of the matrix are one larger than the lengths of
  // corresponding strings.
  //
  std::vector<std::vector<EditNode> > edit_matrix;
  for (unsigned i = 0; i < to_length+ 1; i++)
    edit_matrix.emplace_back (from_length + 1);

  // The initial row of EDIT_MATRIX corresponds to deleting
  // everything in FROM to get a zero-length string.
  //
  for (unsigned from_idx = 0; from_idx < from_length; from_idx++)
    edit_matrix[0][from_idx + 1] = EditNode (DELETE, 0, (from_idx + 1) * costs[DELETE]);

  // Now scan through the matrix a row at a time, filling in each node
  // using the optimal choice from the three available predecessors,
  // and inserting, deleting, or changing/skipping a character.
  //
  for (unsigned to_idx = 0; to_idx < to_length; to_idx++)
    {
      // The first entry in each row is always an insertion, as
      // there's no other choice (because the from string has zero
      // length).
      //
      edit_matrix[to_idx + 1][0] = EditNode (INSERT, to[to_idx], (to_idx + 1) * costs[INSERT]);

      for (unsigned from_idx = 0; from_idx < from_length; from_idx++)
	{
	  EditType rep_type = (from[from_idx] == to[to_idx]) ? SKIP : REPLACE;

	  unsigned ins_cost = edit_matrix[to_idx - 1 + 1][from_idx + 1].cost + costs[INSERT];
	  unsigned del_cost = edit_matrix[to_idx + 1][from_idx - 1 + 1].cost + costs[DELETE];
	  unsigned rep_cost = edit_matrix[to_idx - 1 + 1][from_idx - 1 + 1].cost + costs[rep_type];

	  unsigned cost;
	  EditType type;
	  if (ins_cost > del_cost && ins_cost > rep_cost)
	    {
	      cost = ins_cost;
	      type = INSERT;
	    }
	  else if (del_cost > rep_cost)
	    {
	      cost = del_cost;
	      type = DELETE;
	    }
	  else
	    {
	      cost = rep_cost;
	      type = rep_type;
	    }

	  edit_matrix[to_idx + 1][from_idx + 1] = EditNode (type, to[to_idx], cost);
	}
    }

  // Now that we've computed all the optimal paths, replace the one
  // which reaches the final result.  We start replaying from the
  // final position.
  //
  std::list<Edit> result;
  unsigned from_idx = from_length, to_idx = to_length;
  while (from_idx > 0 || to_idx > 0)
    {
      const Edit &edit = edit_matrix[to_idx + 1][from_idx + 1].edit;
      result.push_front (edit);
      if (edit.type != INSERT)
	from_idx--;
      if (edit.type != DELETE)
	to_idx--;
    }

  return result;
}


int main (int argc, const char **argv)
{
  if (argc != 3)
    {
      std::cerr << "Usage: " << argv[0] << " FROM TO\n";
      return 1;
    }

  std::list<Edit> edits = compute_optimal_edits (argv[1], argv[2], std_edit_costs);

  for (const Edit &edit : edits)
    {
      std::cout << edit_type_names[edit.type] << " " << edit.ch << '\n';
    }
}
