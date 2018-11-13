// Copyright (c) 2018 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SOURCE_OPT_IR_BUILDER_H_
#define SOURCE_OPT_IR_BUILDER_H_

#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "source/opt/basic_block.h"
#include "source/opt/constants.h"
#include "source/opt/instruction.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace opt {

// In SPIR-V, ids are encoded as uint16_t, this id is guaranteed to be always
// invalid.
const uint32_t kInvalidId = std::numeric_limits<uint32_t>::max();

// Helper class to abstract instruction construction and insertion.
// The instruction builder can preserve the following analyses (specified via
// the constructors):
//   - Def-use analysis
//   - Instruction to block analysis
class InstructionBuilder
{
public:
	using InsertionPointTy = BasicBlock::iterator;

	// Creates an InstructionBuilder, all new instructions will be inserted before
	// the instruction |insert_before|.
	InstructionBuilder(
			IRContext* context, Instruction* insert_before,
			IRContext::Analysis preserved_analyses = IRContext::kAnalysisNone)
			: InstructionBuilder(context, context->get_instr_block(insert_before),
								 InsertionPointTy(insert_before),
								 preserved_analyses) {}

	// Creates an InstructionBuilder, all new instructions will be inserted at the
	// end of the basic block |parent_block|.
	InstructionBuilder(
			IRContext* context, BasicBlock* parent_block,
			IRContext::Analysis preserved_analyses = IRContext::kAnalysisNone)
			: InstructionBuilder(context, parent_block, parent_block->end(),
								 preserved_analyses) {}

	// Creates a new selection merge instruction.
	// The id |merge_id| is the merge basic block id.
	Instruction* AddSelectionMerge(
			uint32_t merge_id,
			uint32_t selection_control = SpvSelectionControlMaskNone)
	{
		std::unique_ptr<Instruction> new_branch_merge(new Instruction(
				GetContext(), SpvOpSelectionMerge, 0, 0,
				{{spv_operand_type_t::SPV_OPERAND_TYPE_ID, {merge_id}},
				 {spv_operand_type_t::SPV_OPERAND_TYPE_SELECTION_CONTROL,
														   {selection_control}}}));
		return AddInstruction(std::move(new_branch_merge));
	}

	// Creates a new loop merge instruction.
	// The id |merge_id| is the basic block id of the merge block.
	// |continue_id| is the id of the continue block.
	// |loop_control| are the loop control flags to be added to the instruction.
	Instruction* AddLoopMerge(uint32_t merge_id, uint32_t continue_id,
							  uint32_t loop_control = SpvLoopControlMaskNone)
	{
		std::unique_ptr<Instruction> new_branch_merge(new Instruction(
				GetContext(), SpvOpLoopMerge, 0, 0,
				{{spv_operand_type_t::SPV_OPERAND_TYPE_ID,           {merge_id}},
				 {spv_operand_type_t::SPV_OPERAND_TYPE_ID,           {continue_id}},
				 {spv_operand_type_t::SPV_OPERAND_TYPE_LOOP_CONTROL, {loop_control}}}));
		return AddInstruction(std::move(new_branch_merge));
	}

	// Creates a new branch instruction to |label_id|.
	// Note that the user must make sure the final basic block is
	// well formed.
	Instruction* AddBranch(uint32_t label_id)
	{
		std::unique_ptr<Instruction> new_branch(new Instruction(
				GetContext(), SpvOpBranch, 0, 0,
				{{spv_operand_type_t::SPV_OPERAND_TYPE_ID, {label_id}}}));
		return AddInstruction(std::move(new_branch));
	}

	// Creates a new conditional instruction and the associated selection merge
	// instruction if requested.
	// The id |cond_id| is the id of the condition instruction, must be of
	// type bool.
	// The id |true_id| is the id of the basic block to branch to if the condition
	// is true.
	// The id |false_id| is the id of the basic block to branch to if the
	// condition is false.
	// The id |merge_id| is the id of the merge basic block for the selection
	// merge instruction. If |merge_id| equals kInvalidId then no selection merge
	// instruction will be created.
	// The value |selection_control| is the selection control flag for the
	// selection merge instruction.
	// Note that the user must make sure the final basic block is
	// well formed.
	Instruction* AddConditionalBranch(
			uint32_t cond_id, uint32_t true_id, uint32_t false_id,
			uint32_t merge_id = kInvalidId,
			uint32_t selection_control = SpvSelectionControlMaskNone)
	{
		if(merge_id != kInvalidId)
		{
			AddSelectionMerge(merge_id, selection_control);
		}
		std::unique_ptr<Instruction> new_branch(new Instruction(
				GetContext(), SpvOpBranchConditional, 0, 0,
				{{spv_operand_type_t::SPV_OPERAND_TYPE_ID, {cond_id}},
				 {spv_operand_type_t::SPV_OPERAND_TYPE_ID, {true_id}},
				 {spv_operand_type_t::SPV_OPERAND_TYPE_ID, {false_id}}}));
		return AddInstruction(std::move(new_branch));
	}

	// Creates a new switch instruction and the associated selection merge
	// instruction if requested.
	// The id |selector_id| is the id of the selector instruction, must be of
	// type int.
	// The id |default_id| is the id of the default basic block to branch to.
	// The vector |targets| is the pair of literal/branch id.
	// The id |merge_id| is the id of the merge basic block for the selection
	// merge instruction. If |merge_id| equals kInvalidId then no selection merge
	// instruction will be created.
	// The value |selection_control| is the selection control flag for the
	// selection merge instruction.
	// Note that the user must make sure the final basic block is
	// well formed.
	Instruction* AddSwitch(
			uint32_t selector_id, uint32_t default_id,
			const std::vector<std::pair<Operand::OperandData, uint32_t>>& targets,
			uint32_t merge_id = kInvalidId,
			uint32_t selection_control = SpvSelectionControlMaskNone)
	{
		if(merge_id != kInvalidId)
		{
			AddSelectionMerge(merge_id, selection_control);
		}
		std::vector<Operand> operands;
		operands.emplace_back(
				Operand{spv_operand_type_t::SPV_OPERAND_TYPE_ID, {selector_id}});
		operands.emplace_back(
				Operand{spv_operand_type_t::SPV_OPERAND_TYPE_ID, {default_id}});
		for(auto& target : targets)
		{
			operands.emplace_back(
					Operand{spv_operand_type_t::SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER,
							target.first});
			operands.emplace_back(
					Operand{spv_operand_type_t::SPV_OPERAND_TYPE_ID, {target.second}});
		}
		std::unique_ptr<Instruction> new_switch(
				new Instruction(GetContext(), SpvOpSwitch, 0, 0, operands));
		return AddInstruction(std::move(new_switch));
	}

	// Creates a phi instruction.
	// The id |type| must be the id of the phi instruction's type.
	// The vector |incomings| must be a sequence of pairs of <definition id,
	// parent id>.
	Instruction* AddPhi(uint32_t type, const std::vector<uint32_t>& incomings)
	{
		assert(incomings.size() % 2 == 0 && "A sequence of pairs is expected");
		std::vector<Operand> phi_ops;
		for(size_t i = 0; i < incomings.size(); i++)
		{
			phi_ops.push_back({SPV_OPERAND_TYPE_ID, {incomings[i]}});
		}
		std::unique_ptr<Instruction> phi_inst(new Instruction(
				GetContext(), SpvOpPhi, type, GetContext()->TakeNextId(), phi_ops));
		return AddInstruction(std::move(phi_inst));
	}

	// Creates an addition instruction.
	// The id |type| must be the id of the instruction's type, must be the same as
	// |op1| and |op2| types.
	// The id |op1| is the left hand side of the operation.
	// The id |op2| is the right hand side of the operation.
	Instruction* AddIAdd(uint32_t type, uint32_t op1, uint32_t op2)
	{
		std::unique_ptr<Instruction> inst(new Instruction(
				GetContext(), SpvOpIAdd, type, GetContext()->TakeNextId(),
				{{SPV_OPERAND_TYPE_ID, {op1}},
				 {SPV_OPERAND_TYPE_ID, {op2}}}));
		return AddInstruction(std::move(inst));
	}

	// Creates a less than instruction for unsigned integer.
	// The id |op1| is the left hand side of the operation.
	// The id |op2| is the right hand side of the operation.
	// It is assumed that |op1| and |op2| have the same underlying type.
	Instruction* AddULessThan(uint32_t op1, uint32_t op2)
	{
		analysis::Bool bool_type;
		uint32_t type = GetContext()->get_type_mgr()->GetId(&bool_type);
		std::unique_ptr<Instruction> inst(new Instruction(
				GetContext(), SpvOpULessThan, type, GetContext()->TakeNextId(),
				{{SPV_OPERAND_TYPE_ID, {op1}},
				 {SPV_OPERAND_TYPE_ID, {op2}}}));
		return AddInstruction(std::move(inst));
	}

	// Creates a less than instruction for signed integer.
	// The id |op1| is the left hand side of the operation.
	// The id |op2| is the right hand side of the operation.
	// It is assumed that |op1| and |op2| have the same underlying type.
	Instruction* AddSLessThan(uint32_t op1, uint32_t op2)
	{
		analysis::Bool bool_type;
		uint32_t type = GetContext()->get_type_mgr()->GetId(&bool_type);
		std::unique_ptr<Instruction> inst(new Instruction(
				GetContext(), SpvOpSLessThan, type, GetContext()->TakeNextId(),
				{{SPV_OPERAND_TYPE_ID, {op1}},
				 {SPV_OPERAND_TYPE_ID, {op2}}}));
		return AddInstruction(std::move(inst));
	}

	// Creates an OpILessThan or OpULessThen instruction depending on the sign of
	// |op1|. The id |op1| is the left hand side of the operation. The id |op2| is
	// the right hand side of the operation. It is assumed that |op1| and |op2|
	// have the same underlying type.
	Instruction* AddLessThan(uint32_t op1, uint32_t op2)
	{
		Instruction* op1_insn = context_->get_def_use_mgr()->GetDef(op1);
		analysis::Type* type =
				GetContext()->get_type_mgr()->GetType(op1_insn->type_id());
		analysis::Integer* int_type = type->AsInteger();
		assert(int_type && "Operand is not of int type");

		if(int_type->IsSigned())
			return AddSLessThan(op1, op2);
		else
			return AddULessThan(op1, op2);
	}

	// Creates a select instruction.
	// |type| must match the types of |true_value| and |false_value|. It is up to
	// the caller to ensure that |cond| is a correct type (bool or vector of
	// bool) for |type|.
	Instruction* AddSelect(uint32_t type, uint32_t cond, uint32_t true_value,
						   uint32_t false_value)
	{
		std::unique_ptr<Instruction> select(new Instruction(
				GetContext(), SpvOpSelect, type, GetContext()->TakeNextId(),
				std::initializer_list<Operand>{{SPV_OPERAND_TYPE_ID, {cond}},
											   {SPV_OPERAND_TYPE_ID, {true_value}},
											   {SPV_OPERAND_TYPE_ID, {false_value}}}));
		return AddInstruction(std::move(select));
	}

	// Adds a signed int32 constant to the binary.
	// The |value| parameter is the constant value to be added.
	Instruction* Add32BitSignedIntegerConstant(int32_t value)
	{
		return Add32BitConstantInteger<int32_t>(value, true);
	}

	// Create a composite construct.
	// |type| should be a composite type and the number of elements it has should
	// match the size od |ids|.
	Instruction* AddCompositeConstruct(uint32_t type,
									   const std::vector<uint32_t>& ids)
	{
		std::vector<Operand> ops;
		for(auto id : ids)
		{
			ops.emplace_back(SPV_OPERAND_TYPE_ID,
							 std::initializer_list<uint32_t>{id});
		}
		std::unique_ptr<Instruction> construct(
				new Instruction(GetContext(), SpvOpCompositeConstruct, type,
								GetContext()->TakeNextId(), ops));
		return AddInstruction(std::move(construct));
	}

	// Adds an unsigned int32 constant to the binary.
	// The |value| parameter is the constant value to be added.
	Instruction* Add32BitUnsignedIntegerConstant(uint32_t value)
	{
		return Add32BitConstantInteger<uint32_t>(value, false);
	}

	// Adds either a signed or unsigned 32 bit integer constant to the binary
	// depedning on the |sign|. If |sign| is true then the value is added as a
	// signed constant otherwise as an unsigned constant. If |sign| is false the
	// value must not be a negative number.
	template<typename T>
	Instruction* Add32BitConstantInteger(T value, bool sign)
	{
		// Assert that we are not trying to store a negative number in an unsigned
		// type.
		if(!sign)
			assert(value >= 0 &&
				   "Trying to add a signed integer with an unsigned type!");

		analysis::Integer int_type{32, sign};

		// Get or create the integer type. This rebuilds the type and manages the
		// memory for the rebuilt type.
		uint32_t type_id =
				GetContext()->get_type_mgr()->GetTypeInstruction(&int_type);

		// Get the memory managed type so that it is safe to be stored by
		// GetConstant.
		analysis::Type* rebuilt_type =
				GetContext()->get_type_mgr()->GetType(type_id);

		// Even if the value is negative we need to pass the bit pattern as a
		// uint32_t to GetConstant.
		uint32_t word = value;

		// Create the constant value.
		const analysis::Constant* constant =
				GetContext()->get_constant_mgr()->GetConstant(rebuilt_type, {word});

		// Create the OpConstant instruction using the type and the value.
		return GetContext()->get_constant_mgr()->GetDefiningInstruction(constant);
	}

	Instruction* AddCompositeExtract(uint32_t type, uint32_t id_of_composite,
									 const std::vector<uint32_t>& index_list)
	{
		std::vector<Operand> operands;
		operands.push_back({SPV_OPERAND_TYPE_ID, {id_of_composite}});

		for(uint32_t index : index_list)
		{
			operands.push_back({SPV_OPERAND_TYPE_LITERAL_INTEGER, {index}});
		}

		std::unique_ptr<Instruction> new_inst(
				new Instruction(GetContext(), SpvOpCompositeExtract, type,
								GetContext()->TakeNextId(), operands));
		return AddInstruction(std::move(new_inst));
	}

	// Creates an unreachable instruction.
	Instruction* AddUnreachable()
	{
		std::unique_ptr<Instruction> select(
				new Instruction(GetContext(), SpvOpUnreachable, 0, 0,
								std::initializer_list<Operand>{}));
		return AddInstruction(std::move(select));
	}

	Instruction* AddAccessChain(uint32_t type_id, uint32_t base_ptr_id,
								std::vector<uint32_t> ids)
	{
		std::vector<Operand> operands;
		operands.push_back({SPV_OPERAND_TYPE_ID, {base_ptr_id}});

		for(uint32_t index_id : ids)
		{
			operands.push_back({SPV_OPERAND_TYPE_ID, {index_id}});
		}

		std::unique_ptr<Instruction> new_inst(
				new Instruction(GetContext(), SpvOpAccessChain, type_id,
								GetContext()->TakeNextId(), operands));
		return AddInstruction(std::move(new_inst));
	}

	Instruction* AddLoad(uint32_t type_id, uint32_t base_ptr_id)
	{
		std::vector<Operand> operands;
		operands.push_back({SPV_OPERAND_TYPE_ID, {base_ptr_id}});

		std::unique_ptr<Instruction> new_inst(
				new Instruction(GetContext(), SpvOpLoad, type_id,
								GetContext()->TakeNextId(), operands));
		return AddInstruction(std::move(new_inst));
	}

	// Inserts the new instruction before the insertion point.
	Instruction* AddInstruction(std::unique_ptr<Instruction>&& insn)
	{
		Instruction* insn_ptr = &*insert_before_.InsertBefore(std::move(insn));
		UpdateInstrToBlockMapping(insn_ptr);
		UpdateDefUseMgr(insn_ptr);
		return insn_ptr;
	}

	// Returns the insertion point iterator.
	InsertionPointTy GetInsertPoint() { return insert_before_; }

	// Change the insertion point to insert before the instruction
	// |insert_before|.
	void SetInsertPoint(Instruction* insert_before)
	{
		parent_ = context_->get_instr_block(insert_before);
		insert_before_ = InsertionPointTy(insert_before);
	}

	// Change the insertion point to insert at the end of the basic block
	// |parent_block|.
	void SetInsertPoint(BasicBlock* parent_block)
	{
		parent_ = parent_block;
		insert_before_ = parent_block->end();
	}

	// Returns the context which instructions are constructed for.
	IRContext* GetContext() const { return context_; }

	// Returns the set of preserved analyses.
	inline IRContext::Analysis GetPreservedAnalysis() const
	{
		return preserved_analyses_;
	}

private:
	InstructionBuilder(IRContext* context, BasicBlock* parent,
					   InsertionPointTy insert_before,
					   IRContext::Analysis preserved_analyses)
			: context_(context),
			  parent_(parent),
			  insert_before_(insert_before),
			  preserved_analyses_(preserved_analyses)
	{
		assert(!(preserved_analyses_ & ~(IRContext::kAnalysisDefUse |
										 IRContext::kAnalysisInstrToBlockMapping)));
	}

	// Returns true if the users requested to update |analysis|.
	inline bool IsAnalysisUpdateRequested(IRContext::Analysis analysis) const
	{
		return preserved_analyses_ & analysis;
	}

	// Updates the def/use manager if the user requested it. If he did not request
	// an update, this function does nothing.
	inline void UpdateDefUseMgr(Instruction* insn)
	{
		if(IsAnalysisUpdateRequested(IRContext::kAnalysisDefUse))
			GetContext()->get_def_use_mgr()->AnalyzeInstDefUse(insn);
	}

	// Updates the instruction to block analysis if the user requested it. If he
	// did not request an update, this function does nothing.
	inline void UpdateInstrToBlockMapping(Instruction* insn)
	{
		if(IsAnalysisUpdateRequested(IRContext::kAnalysisInstrToBlockMapping) &&
		   parent_)
			GetContext()->set_instr_block(insn, parent_);
	}

	IRContext* context_;
	BasicBlock* parent_;
	InsertionPointTy insert_before_;
	const IRContext::Analysis preserved_analyses_;
};

}  // namespace opt
}  // namespace spvtools

#endif  // SOURCE_OPT_IR_BUILDER_H_
