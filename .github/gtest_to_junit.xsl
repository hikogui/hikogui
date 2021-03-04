<?xml version="1.0" encoding="UTF-8"?>
<!--
Copyright (c) 2021, Jens A. Koch.
License: BSL-1.0, see https://opensource.org/licenses/BSL-1.0
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="xml" indent="yes" />

    <xsl:template match="/">
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="//testsuites">
        <testsuites>
            <xsl:apply-templates/>
        </testsuites>
    </xsl:template>

    <xsl:template match="//testsuite">
        <testsuite>
            <xsl:attribute name="name">     <xsl:value-of select="@name"/></xsl:attribute>
            <xsl:attribute name="tests">    <xsl:value-of select="@tests"/></xsl:attribute>
            <xsl:attribute name="failures"> <xsl:value-of select="@failures"/></xsl:attribute>
            <xsl:attribute name="errors">   <xsl:value-of select="@errors"/></xsl:attribute>
            <xsl:attribute name="skipped">  <xsl:value-of select="@disabled + count(testcase[@result = 'skipped'])"/></xsl:attribute>
            <xsl:attribute name="time">     <xsl:value-of select="@time"/></xsl:attribute>
            <xsl:apply-templates select="testcase"/>
        </testsuite>
    </xsl:template>

    <xsl:template match="//testcase">
        <testcase>
            <xsl:choose>
                <xsl:when test="@value_param">
                    <xsl:attribute name="name">
                        <xsl:value-of select="@name"/> (<xsl:value-of select="@value_param"/>)
                    </xsl:attribute>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:attribute name="name">
                        <xsl:value-of select="@name"/>
                    </xsl:attribute>
                </xsl:otherwise>
            </xsl:choose>
            <xsl:attribute name="time">
                <xsl:value-of select="@time"/>
            </xsl:attribute>
            <xsl:attribute name="classname">
                <xsl:value-of select="@classname"/>
            </xsl:attribute>
            <xsl:if test="@status = 'notrun' or @result = 'skipped'">
                <skipped/>
            </xsl:if>
            <xsl:if test="failure">
                <failure>
                    <xsl:for-each select="failure">
                        <xsl:if test="not(position()=1)">
                            <xsl:text>&#xa;&#xa;</xsl:text>
                        </xsl:if>
                        <xsl:value-of select="@message"/>
                    </xsl:for-each>
                </failure>
                <system-out>
                    <xsl:for-each select="failure">
                        <xsl:if test="not(position()=1)">
                            <xsl:text>&#xa;&#xa;</xsl:text>
                        </xsl:if>
                        <xsl:value-of select="."/>
                    </xsl:for-each>
                </system-out>
            </xsl:if>
			<xsl:if test="error">
                <failure>
                    <xsl:for-each select="error">
                        <xsl:if test="not(position()=1)">
                            <xsl:text>&#xa;&#xa;</xsl:text>
                        </xsl:if>
                        <xsl:value-of select="@message"/>
                    </xsl:for-each>
                </failure>
                <system-out>
                    <xsl:for-each select="error">
                        <xsl:if test="not(position()=1)">
                            <xsl:text>&#xa;&#xa;</xsl:text>
                        </xsl:if>
                        <xsl:value-of select="."/>
                    </xsl:for-each>
                </system-out>
            </xsl:if>
        </testcase>
    </xsl:template>

</xsl:stylesheet>