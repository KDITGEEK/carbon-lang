// Part of the Carbon Language project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "toolchain/semantics/semantics_context.h"
#include "toolchain/semantics/semantics_node.h"

namespace Carbon {

auto SemanticsHandleMemberAccessExpression(SemanticsContext& context,
                                           ParseTree::Node parse_node) -> bool {
  SemanticsStringId name_id = context.node_stack().Pop<ParseNodeKind::Name>();

  auto base_id = context.node_stack().PopExpression();
  auto base = context.semantics_ir().GetNode(base_id);
  if (base.kind() == SemanticsNodeKind::Namespace) {
    // For a namespace, just resolve the name.
    auto node_id =
        context.LookupName(parse_node, name_id, base.GetAsNamespace(),
                           /*print_diagnostics=*/true);
    context.node_stack().Push(parse_node, node_id);
    return true;
  }

  auto base_type = context.semantics_ir().GetNode(
      context.semantics_ir().GetTypeAllowBuiltinTypes(base.type_id()));

  switch (base_type.kind()) {
    case SemanticsNodeKind::StructType: {
      auto refs =
          context.semantics_ir().GetNodeBlock(base_type.GetAsStructType());
      // TODO: Do we need to optimize this with a lookup table for O(1)?
      for (int i = 0; i < static_cast<int>(refs.size()); ++i) {
        auto ref = context.semantics_ir().GetNode(refs[i]);
        if (auto [field_name_id, field_type_id] = ref.GetAsStructTypeField();
            name_id == field_name_id) {
          context.AddNodeAndPush(
              parse_node,
              SemanticsNode::StructMemberAccess::Make(
                  parse_node, field_type_id, base_id, SemanticsMemberIndex(i)));
          return true;
        }
      }
      CARBON_DIAGNOSTIC(QualifiedExpressionNameNotFound, Error,
                        "Type `{0}` does not have a member `{1}`.", std::string,
                        llvm::StringRef);
      context.emitter().Emit(
          parse_node, QualifiedExpressionNameNotFound,
          context.semantics_ir().StringifyType(base.type_id()),
          context.semantics_ir().GetString(name_id));
      break;
    }
    default: {
      if (base.type_id() != SemanticsTypeId::Error) {
        CARBON_DIAGNOSTIC(QualifiedExpressionUnsupported, Error,
                          "Type `{0}` does not support qualified expressions.",
                          std::string);
        context.emitter().Emit(
            parse_node, QualifiedExpressionUnsupported,
            context.semantics_ir().StringifyType(base.type_id()));
      }
      break;
    }
  }

  // Should only be reached on error.
  context.node_stack().Push(parse_node, SemanticsNodeId::BuiltinError);
  return true;
}

auto SemanticsHandlePointerMemberAccessExpression(SemanticsContext& context,
                                                  ParseTree::Node parse_node)
    -> bool {
  return context.TODO(parse_node, "HandlePointerMemberAccessExpression");
}

auto SemanticsHandleName(SemanticsContext& context, ParseTree::Node parse_node)
    -> bool {
  auto name_str = context.parse_tree().GetNodeText(parse_node);
  auto name_id = context.semantics_ir().AddString(name_str);
  // The parent is responsible for binding the name.
  context.node_stack().Push(parse_node, name_id);
  return true;
}

auto SemanticsHandleNameExpression(SemanticsContext& context,
                                   ParseTree::Node parse_node) -> bool {
  auto name_str = context.parse_tree().GetNodeText(parse_node);
  auto name_id = context.semantics_ir().AddString(name_str);
  context.node_stack().Push(
      parse_node,
      context.LookupName(parse_node, name_id, SemanticsNameScopeId::Invalid,
                         /*print_diagnostics=*/true));
  return true;
}

auto SemanticsHandleQualifiedDeclaration(SemanticsContext& context,
                                         ParseTree::Node parse_node) -> bool {
  auto pop_and_apply_first_child = [&]() {
    if (context.parse_tree().node_kind(context.node_stack().PeekParseNode()) !=
        ParseNodeKind::QualifiedDeclaration) {
      // First QualifiedDeclaration in a chain.
      auto [parse_node1, node_id1] =
          context.node_stack().PopExpressionWithParseNode();
      context.declaration_name_stack().ApplyExpressionQualifier(parse_node1,
                                                                node_id1);
      // Add the QualifiedDeclaration so that it can be used for bracketing.
      context.node_stack().Push(parse_node);
    } else {
      // Nothing to do: the QualifiedDeclaration remains as a bracketing node
      // for later QualifiedDeclarations.
    }
  };

  ParseTree::Node parse_node2 = context.node_stack().PeekParseNode();
  if (context.parse_tree().node_kind(parse_node2) == ParseNodeKind::Name) {
    SemanticsStringId name_id2 =
        context.node_stack().Pop<ParseNodeKind::Name>();
    pop_and_apply_first_child();
    context.declaration_name_stack().ApplyNameQualifier(parse_node2, name_id2);
  } else {
    SemanticsNodeId node_id2 = context.node_stack().PopExpression();
    pop_and_apply_first_child();
    context.declaration_name_stack().ApplyExpressionQualifier(parse_node2,
                                                              node_id2);
  }

  return true;
}

auto SemanticsHandleSelfTypeNameExpression(SemanticsContext& context,
                                           ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleSelfTypeNameExpression");
}

auto SemanticsHandleSelfValueName(SemanticsContext& context,
                                  ParseTree::Node parse_node) -> bool {
  return context.TODO(parse_node, "HandleSelfValueName");
}

}  // namespace Carbon
