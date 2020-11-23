/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pablo <pablo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/01 19:52:58 by pablo             #+#    #+#             */
/*   Updated: 2020/11/23 14:34:55 by pablo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <execution.h>
#include <builtins.h>
#include <expansion.h>
#include <job_control.h>
#include <errors.h>

static t_exec_status	get_exec(t_exec* info, t_term* term)
{
	const char	*names[] = {"echo", "cd", "pwd", "export", "unset", "env", \
			"exit", "fg", "jobs", "kill", "bg", "disown", "wait"};
	const int	lengths[] = {4, 3, 4, 7, 5, 4, 5, 2, 4, 4, 2, 6, 4};
	const t_executable 	builtins[] = {&ft_echo, &ft_cd, &ft_pwd, &ft_export, \
			&ft_unset, &ft_env, &ft_exit, &ft_fg, &ft_jobs, &ft_kill, &ft_bg, &ft_disown, &ft_wait};
	size_t			i;

	int status = SUCCESS;
	i = 0;
	while (i < sizeof(names) / sizeof(*names) && ft_strncmp(info->av[0], names[i], lengths[i]))
		i++;
	if (i < sizeof(names) / sizeof(*names))
		info->exec = builtins[i];
	else if ((status = build_execve_args(info, term)) == SUCCESS)
		info->exec = &execute_child;
	return (status);
}

static t_exec_status	execute_cmd(t_bst* cmd, t_exec* info, t_term* term)
{
	char**				filename;
	t_redir_status		redir_st;
	t_exec_status		exec_st;

	if ((redir_st = redirections_handler(&info, cmd, term, &filename)) != CONTINUE)
		return (print_redirection_error(redir_st, filename, term));
	if (!(cmd->type & CMD) || (cmd->type & PIPE \
			&& !(((t_bst*)cmd->a)->type & CMD)))
    	exec_st = execute_cmd(cmd->a, info, term);
	else
	{
		if (!(info->av = tokens_expand((t_tok**)&cmd->a, &term->env, &info->ac)))
			return (RDR_BAD_ALLOC);
		if (!info->av[0])
			return (SUCCESS);
		;
		if ((exec_st = get_exec(info, term)) == SUCCESS)
			term->st = info->exec(info, term);
		else
			destroy_execve_args(info);
		if (PRINT_DEBUG)
			ft_dprintf(2, "[EXEC CMD][CURR PROCESS FLAGS: %d][\'%p\']\n", g_session->groups->active_processes->flags, g_session->groups->active_processes);
		if (close_pipe_fds(info->fds) != SUCCESS)
			return (BAD_CLOSE);// return error code (could not BAD_CLOSE to define later)
	}
	return (exec_st == BAD_PATH ? SUCCESS : exec_st);
}

static t_exec_status	execute_job(t_bst* job, t_exec* info, t_term* term)
{
	t_exec_status		st;

	st = SUCCESS; // TODO: init
	info->handle_dup = NONE;
	if (open_pipe_fds(&info, job->b ? job->type : 0) != SUCCESS)
		return (BAD_PIPE);
	if (!(job->type & (CMD | REDIR_DG | REDIR_GR | REDIR_LE)))
    	st = execute_cmd(job->a, info, term);
    if (st == SUCCESS && job->b && job->type & PIPE)
        st = execute_job(job->b, info, term);
	else if (st == SUCCESS) // can i put this else in the second "if" as a ternary ?
		st = execute_cmd(job, info, term);
	return (st);
}

t_exec_status			execute_bst(t_bst* root, t_term* term)
{
	t_exec				info;
	t_exec_status		st;
	t_group*			group;

	// New stuff
	remove_exited_zombies();
	if (!(group = group_new()))
		return (BAD_ALLOC);
/*	if (PRINT_DEBUG) {
	if (g_session->groups && g_session->groups->active_processes)
	{
		ft_dprintf(2, "[XXXXXXXXXXXXXXXXX : %p]\n", g_session->groups->active_processes);
		if (g_session->groups->next && g_session->groups->next->active_processes)
			ft_dprintf(2, "[EXEC BST][PREVIOUS GROUP LEADER FLAGS ARE: %d][\'%p\']\n", g_session->groups->next->active_processes->flags, g_session->groups->next->active_processes);
	}
	}*/
	group_push_front(group);

	ft_bzero(&info, sizeof(t_exec));
	info = (t_exec){.fds[FDS_STDOUT]=FDS_STDOUT, .fds[FDS_AUX]=FDS_AUX};
	if (root->type & PIPE)
		st = execute_job(root, &info, term);
	else
		st = execute_cmd(root, &info, term);
	return (wait_processes_v2(term, st));
}
